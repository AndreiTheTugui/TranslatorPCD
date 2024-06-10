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
#define MAX_BANNED_IPS 100

int translation_service_running = 0;
pthread_t translation_service_thread;
char banned_ips[MAX_BANNED_IPS][BUFFER_SIZE];
int banned_ip_count = 0;

void* translation_service(void* arg) {
    printf("Translation service started.\n");
    while (translation_service_running) {
        sleep(1);
    }
    printf("Translation service stopped.\n");
    return NULL;
}

bool is_banned(const char* ip_address) {
    for (int i = 0; i < banned_ip_count; i++) {
        if (strcmp(banned_ips[i], ip_address) == 0) {
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
        
    } else if (strcmp(command, "stop") == 0) {
        
    } else if (strncmp(command, "ban_ip ", 7) == 0) {
        const char* ip_to_ban = command + 7; // Skip "ban_ip "
        if (banned_ip_count < MAX_BANNED_IPS) {
            strncpy(banned_ips[banned_ip_count], ip_to_ban, BUFFER_SIZE);
            banned_ip_count++;
            snprintf(response, BUFFER_SIZE, "IP %s banned.", ip_to_ban);
        } else {
            snprintf(response, BUFFER_SIZE, "Maximum number of banned IPs reached.");
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

        handle_admin_command(buffer, response);
        send(new_socket, response, strlen(response), 0);

        close(new_socket);
    }

    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}
