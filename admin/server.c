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
#define BUFFER_SIZE 1024
#define LOCALE_DIR "locale"
#define DOMAIN "messages"

void *handle_client(void *client_socket);

void translate_text(const char *input, char *output);

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    pthread_t tid;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Server socket created successfully\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Socket binding failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket binding successful\n");

    if (listen(server_socket, 10) < 0) {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d\n", PORT);

    while (1) {
        addr_size = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
        if (client_socket < 0) {
            perror("Connection accept failed");
            exit(EXIT_FAILURE);
        }
        printf("Client connected\n");

        if (pthread_create(&tid, NULL, handle_client, (void *)&client_socket) != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }

    close(server_socket);
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
        pthread_exit(NULL);
    }

    fread(buffer, sizeof(char), BUFFER_SIZE, file);
    fclose(file);

    translate_text(buffer, translated_text);

    file = fopen("translated.txt", "w");
    if (file == NULL) {
        perror("File open failed");
        close(sock);
        pthread_exit(NULL);
    }
    fwrite(translated_text, sizeof(char), strlen(translated_text), file);
    fclose(file);

    write(sock, "translated.txt", strlen("translated.txt") + 1);

    close(sock);
    pthread_exit(NULL);
}

void translate_text(const char *input, char *output) {
    setlocale(LC_ALL, "");
    bindtextdomain(DOMAIN, LOCALE_DIR);
    textdomain(DOMAIN);

    strcpy(output, gettext(input));
}

