#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <locale.h>
#include <libintl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define PORT 12345
#define ADMIN_PORT 12346
#define BUFFER_SIZE 1024
#define LOCALE_DIR "locale"
#define DOMAIN "messages"

void *handle_client(void *client_socket);
void translate_text(const char *input, char *output);
void *handle_admin_request(void *admin_socket);

int connected_clients = 0;
pthread_mutex_t client_counter_mutex = PTHREAD_MUTEX_INITIALIZER;

int main() {
    int server_socket, admin_socket;
    int client_socket, admin_client_socket;
    struct sockaddr_in server_addr, client_addr, admin_addr;
    socklen_t addr_size;
    pthread_t tid;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Server socket created successfully\n");

    admin_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (admin_socket < 0) {
        perror("Admin socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Admin socket created successfully\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    admin_addr.sin_family = AF_INET;
    admin_addr.sin_port = htons(ADMIN_PORT);
    admin_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Socket binding failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket binding successful\n");

    if (bind(admin_socket, (struct sockaddr *)&admin_addr, sizeof(admin_addr)) < 0) {
        perror("Admin socket binding failed");
        exit(EXIT_FAILURE);
    }
    printf("Admin socket binding successful\n");

    if (listen(server_socket, 10) < 0) {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d\n", PORT);

    if (listen(admin_socket, 10) < 0) {
        perror("Admin listening failed");
        exit(EXIT_FAILURE);
    }
    printf("Admin listening on port %d\n", ADMIN_PORT);

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        FD_SET(admin_socket, &readfds);
        int max_sd = server_socket > admin_socket ? server_socket : admin_socket;

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Select error");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(server_socket, &readfds)) {
            addr_size = sizeof(client_addr);
            client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
            if (client_socket < 0) {
                perror("Connection accept failed");
                exit(EXIT_FAILURE);
            }
            printf("Client connected\n");

            pthread_mutex_lock(&client_counter_mutex);
            connected_clients++;
            pthread_mutex_unlock(&client_counter_mutex);

            if (pthread_create(&tid, NULL, handle_client, (void *)&client_socket) != 0) {
                perror("Thread creation failed");
                exit(EXIT_FAILURE);
            }
        }

        if (FD_ISSET(admin_socket, &readfds)) {
            addr_size = sizeof(admin_addr);
            admin_client_socket = accept(admin_socket, (struct sockaddr *)&admin_addr, &addr_size);
            if (admin_client_socket >= 0) {
                if (pthread_create(&tid, NULL, handle_admin_request, (void *)&admin_client_socket) != 0) {
                    perror("Admin thread creation failed");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    close(server_socket);
    close(admin_socket);
    return 0;
}

void *handle_client(void *client_socket) {
    int sock = *(int *)client_socket;
    char buffer[BUFFER_SIZE];
    char translated_text[BUFFER_SIZE];

    read(sock, buffer, BUFFER_SIZE);
    printf("Received file name: %s\n", buffer);

    FILE *file = fopen(buffer, "r");
    if (file == NULL) {
        perror("File open failed");
        close(sock);
        pthread_mutex_lock(&client_counter_mutex);
        connected_clients--;
        pthread_mutex_unlock(&client_counter_mutex);
        pthread_exit(NULL);
    }

    fread(buffer, sizeof(char), BUFFER_SIZE, file);
    fclose(file);

    translate_text(buffer, translated_text);

    file = fopen("translated.txt", "w");
    if (file == NULL) {
        perror("File open failed");
        close(sock);
        pthread_mutex_lock(&client_counter_mutex);
        connected_clients--;
        pthread_mutex_unlock(&client_counter_mutex);
        pthread_exit(NULL);
    }
    fwrite(translated_text, sizeof(char), strlen(translated_text), file);
    fclose(file);

    write(sock, "translated.txt", strlen("translated.txt") + 1);

    close(sock);
    pthread_mutex_lock(&client_counter_mutex);
    connected_clients--;
    pthread_mutex_unlock(&client_counter_mutex);
    pthread_exit(NULL);
}

void translate_text(const char *input, char *output) {
    setlocale(LC_ALL, "");
    bindtextdomain(DOMAIN, LOCALE_DIR);
    textdomain(DOMAIN);

    strcpy(output, gettext(input));
}

void *handle_admin_request(void *admin_socket) {
    int sock = *(int *)admin_socket;
    char buffer[BUFFER_SIZE];

    recv(sock, buffer, BUFFER_SIZE, 0);

    if (strcmp(buffer, "GET_CLIENT_COUNT") == 0) {
        char client_count[10];
        pthread_mutex_lock(&client_counter_mutex);
        snprintf(client_count, 10, "%d", connected_clients);
        pthread_mutex_unlock(&client_counter_mutex);
        send(sock, client_count, strlen(client_count) + 1, 0);
    }

    close(sock);
    pthread_exit(NULL);
}
