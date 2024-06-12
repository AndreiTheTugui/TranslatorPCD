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

void* handle_client(void* client_socket) {
    int socket = *(int*)client_socket;
    free(client_socket);

    char buffer[BUFFER_SIZE];
    read(socket, buffer, BUFFER_SIZE);
    printf("Received: %s\n", buffer);
    send(socket, "Hello from server", strlen("Hello from server"), 0);

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
    send(socket, "Hello from Unix server", strlen("Hello from Unix server"), 0);

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
        int* client_socket = malloc(sizeof(int));
        *client_socket = new_socket;
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_unix_client, client_socket);
        pthread_detach(thread_id);
    }
}

/*void* handle_soap_client(void* soap_instance) {
    struct soap* soap = (struct soap*)soap_instance;
    soap_serve(soap);
    soap_destroy(soap);
    soap_end(soap);
    soap_free(soap);
    return NULL;
}

void start_soap_server() {
    struct soap soap;
    soap_init(&soap);

    if (!soap_valid_socket(soap_bind(&soap, NULL, 8081, 100))) {
        soap_print_fault(&soap, stderr);
        exit(EXIT_FAILURE);
    }

    while (1) {
        int client_socket = soap_accept(&soap);
        if (!soap_valid_socket(client_socket)) {
            soap_print_fault(&soap, stderr);
            continue;
        }

        struct soap* soap_instance = soap_copy(&soap);
        soap_instance->socket = client_socket;

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_soap_client, soap_instance);
        pthread_detach(thread_id);
    }

    soap_done(&soap);
}*/

int main() {
    pthread_t tcp_thread, unix_thread, soap_thread;

    pthread_create(&tcp_thread, NULL, (void* (*)(void*))start_tcp_server, NULL);
    pthread_create(&unix_thread, NULL, (void* (*)(void*))start_unix_server, NULL);
    //pthread_create(&soap_thread, NULL, (void* (*)(void*))start_soap_server, NULL);

    pthread_join(tcp_thread, NULL);
    pthread_join(unix_thread, NULL);
    //pthread_join(soap_thread, NULL);

    return 0;
}