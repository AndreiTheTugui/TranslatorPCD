#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>

#define ADMIN_PORT 9090
#define BUFFER_SIZE 1024
#define MAX_BANNED_USERS 100

int translation_service_running = 0;
pthread_t translation_service_thread;
char banned_users[MAX_BANNED_USERS][INET_ADDRSTRLEN];
int banned_user_count = 0;

void* translation_service(void* arg) {
    printf("Translation service started.\n");
    while (translation_service_running) {
        sleep(1);
    }
    printf("Translation service stopped.\n");
    return NULL;
}

bool is_banned(const char* ip_address) {
    for (int i = 0; i < banned_user_count; i++) {
        if (strcmp(banned_users[i], ip_address) == 0) {
            return true;
        }
    }
    return false;
}

void handle_admin_command(const char* command, char* response, size_t response_size, const char* client_ip) {
    if (strcmp(command, "status") == 0) {
        snprintf(response, response_size, "Translation service is %s.", 
                 translation_service_running ? "running" : "stopped");
    } else if (strcmp(command, "start") == 0) {
        if (!translation_service_running) {
            translation_service_running = 1;
            pthread_create(&translation_service_thread, NULL, translation_service, NULL);
            snprintf(response, response_size, "Translation service started.");
        } else {
            snprintf(response, response_size, "Translation service is already running.");
        }
    } else if (strcmp(command, "stop") == 0) {
        if (translation_service_running) {
            translation_service_running = 0;
            pthread_join(translation_service_thread, NULL);
            snprintf(response, response_size, "Translation service stopped.");
        } else {
            snprintf(response, response_size, "Translation service is not running.");
        }
    } else if (strncmp(command, "ban ", 4) == 0) {
        const char* ip_to_ban = command + 4;
        if (banned_user_count < MAX_BANNED_USERS) {
            strncpy(banned_users[banned_user_count], ip_to_ban, INET_ADDRSTRLEN);
            banned_user_count++;
            snprintf(response, response_size, "User %s banned.", ip_to_ban);
        } else {
            snprintf(response, response_size, "Banned user list is full.");
        }
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
    char client_ip[INET_ADDRSTRLEN];

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

        inet_ntop(AF_INET, &address.sin_addr, client_ip, INET_ADDRSTRLEN);
        printf("Connection from %s\n", client_ip);

        if (is_banned(client_ip)) {
            snprintf(response, sizeof(response), "Access denied. You are banned.");
            send(new_socket, response, strlen(response), 0);
            close(new_socket);
            continue;
        }

        read(new_socket, buffer, BUFFER_SIZE);
        printf("Received command: %s\n", buffer);

        handle_admin_command(buffer, response, sizeof(response), client_ip);
        send(new_socket, response, strlen(response), 0);

        close(new_socket);
    }

    close(server_fd);
    return 0;
}
