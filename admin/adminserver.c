#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <stdbool.h>

#define SOCKET_PATH "/tmp/admin_socket"
#define BUFFER_SIZE 1024
#define MAX_BANNED_USERS 100

int translation_service_running = 0;
pthread_t translation_service_thread;
char banned_users[MAX_BANNED_USERS][BUFFER_SIZE];
int banned_user_count = 0;

void* translation_service(void* arg) {
    printf("Translation service started.\n");
    while (translation_service_running) {
        sleep(1);
    }
    printf("Translation service stopped.\n");
    return NULL;
}

bool is_banned(const char* user) {
    for (int i = 0; i < banned_user_count; i++) {
        if (strcmp(banned_users[i], user) == 0) {
            return true;
        }
    }
    return false;
}

void handle_admin_command(const char* command, char* response) {
    if (strcmp(command, "status") == 0) {
        snprintf(response, BUFFER_SIZE, "Translation service is %s.", 
                 translation_service_running ? "running" : "stopped");
    } else if (strcmp(command, "start") == 0) {
        if (!translation_service_running) {
            translation_service_running = 1;
            pthread_create(&translation_service_thread, NULL, translation_service, NULL);
            snprintf(response, BUFFER_SIZE, "Translation service started.");
        } else {
            snprintf(response, BUFFER_SIZE, "Translation service is already running.");
        }
    } else if (strcmp(command, "stop") == 0) {
        if (translation_service_running) {
            translation_service_running = 0;
            pthread_join(translation_service_thread, NULL);
            snprintf(response, BUFFER_SIZE, "Translation service stopped.");
        } else {
            snprintf(response, BUFFER_SIZE, "Translation service is not running.");
        }
    } else if (strncmp(command, "ban ", 4) == 0) {
        const char* user_to_ban = command + 4;
        if (banned_user_count < MAX_BANNED_USERS) {
            strncpy(banned_users[banned_user_count], user_to_ban, BUFFER_SIZE);
            banned_user_count++;
            snprintf(response, BUFFER_SIZE, "User %s banned.", user_to_ban);
        } else {
            snprintf(response, BUFFER_SIZE, "Banned user list is full.");
        }
    } else {
        snprintf(response, BUFFER_SIZE, "Unknown command.");
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_un address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE] = {0};
    char *client_command; 

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) - 1);
    unlink(SOCKET_PATH); 

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

    printf("Admin server listening on %s\n", SOCKET_PATH);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept failed");
            continue;
        }

        read(new_socket, buffer, BUFFER_SIZE);
        printf("Received command: %s\n", buffer);

        
        client_command = strtok(buffer, " ");

        handle_admin_command(client_command, response);
        send(new_socket, response, strlen(response), 0);

        close(new_socket);
    }

    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}
