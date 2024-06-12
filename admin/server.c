#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/un.h>

#define SOCKET_PORT 8080
#define BUFFER_SIZE 1024
#define UNIX_SOCKET_PATH "/tmp/translator.sock"
#define MAX_LANGUAGES 10
#define MAX_CLIENTS 100

typedef struct {
    char languages[MAX_LANGUAGES][BUFFER_SIZE];
    int blocked_clients[MAX_CLIENTS];
    int num_languages;
    int max_clients;
} Admin;

Admin admin;
int connected_clients_count = 0; // Track connected clients

void* handle_client(void* client_socket) {
    int socket = *(int*)client_socket;
    free(client_socket);

    char buffer[BUFFER_SIZE];
    read(socket, buffer, BUFFER_SIZE);
    printf("Received: %s\n", buffer);

    // Admin command handling
    if (strcmp(buffer, "show_connected_clients") == 0) {
        char response[BUFFER_SIZE];
        sprintf(response, "%d", connected_clients_count);
        send(socket, response, strlen(response) + 1, 0);
    } else {
        // Handle other commands if needed
        send(socket, "Invalid command", strlen("Invalid command"), 0);
    }

    close(socket);
    return NULL;
}

void start_tcp_server() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(SOCKET_PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        connected_clients_count++; // Increment connected clients count
        int* client_socket = malloc(sizeof(int));
        *client_socket = new_socket;
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, client_socket);
        pthread_detach(thread_id);
    }
}

void* handle_unix_client(void* client_socket) {
    int socket = *(int*)client_socket;
    free(client_socket);

    char buffer[BUFFER_SIZE];
    read(socket, buffer, BUFFER_SIZE);
    printf("Unix socket received: %s\n", buffer);

    // Admin command handling
    if (strcmp(buffer, "show_connected_clients") == 0) {
        char response[BUFFER_SIZE];
        sprintf(response, "%d", connected_clients_count);
        send(socket, response, strlen(response) + 1, 0);
    } else {
        // Handle other commands if needed
        send(socket, "Invalid command", strlen("Invalid command"), 0);
    }

    close(socket);
    return NULL;
}

void start_unix_server() {
    int unix_fd, new_socket;
    struct sockaddr_un address;
    int addrlen = sizeof(address);

    if ((unix_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == 0) {
        perror("unix socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&address, 0, sizeof(struct sockaddr_un));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, UNIX_SOCKET_PATH, sizeof(address.sun_path) - 1);

    unlink(UNIX_SOCKET_PATH);

    if (bind(unix_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(unix_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(unix_fd, 3) < 0) {
        perror("listen");
        close(unix_fd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        if ((new_socket = accept(unix_fd, NULL, NULL)) < 0) {
            perror("accept");
            close(unix_fd);
            exit(EXIT_FAILURE);
        }
        connected_clients_count++; // Increment connected clients count
        int* client_socket = malloc(sizeof(int));
        *client_socket = new_socket;
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_unix_client, client_socket);
        pthread_detach(thread_id);
    }
}

int main() {
    pthread_t tcp_thread, unix_thread;

    // Initialize admin data
    admin.num_languages = 0;
    admin.max_clients = MAX_CLIENTS;
    memset(admin.blocked_clients, 0, sizeof(admin.blocked_clients));

    // Start TCP server thread
    pthread_create(&tcp_thread, NULL, (void* (*)(void*))start_tcp_server, NULL);

    // Start Unix domain socket server thread
    pthread_create(&unix_thread, NULL, (void* (*)(void*))start_unix_server, NULL);

    pthread_join(tcp_thread, NULL);
    pthread_join(unix_thread, NULL);

    return 0;
}

