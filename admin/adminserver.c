#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define ADMIN_PORT 9090
#define BUFFER_SIZE 1024
void handle_admin_command(const char* command, char* response, size_t response_size) {
    if (strcmp(command, "status") == 0) {
        snprintf(response, response_size, "Server is running.");
    } else if (strcmp(command, "stop") == 0) {
        snprintf(response, response_size, "Server will stop.");
    } else {
        snprintf(response, response_size, "Unknown command.");
    }
}
int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(ADMIN_PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Admin server listening on port %d\n", ADMIN_PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept failed");
            continue;
        }

        read(new_socket, buffer, BUFFER_SIZE);
        printf("Received command: %s\n", buffer);

        handle_admin_command(buffer, response, sizeof(response));
        send(new_socket, response, strlen(response), 0);

        close(new_socket);
    }

    close(server_fd);
    return 0;
}
