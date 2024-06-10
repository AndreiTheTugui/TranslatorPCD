#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/admin_socket"
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_un serv_addr;
    char command[BUFFER_SIZE] = {0};
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, SOCKET_PATH, sizeof(serv_addr.sun_path) - 1);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return -1;
    }

    printf("Enter command: ");
    fgets(command, BUFFER_SIZE, stdin); 


    command[strcspn(command, "\n")] = '\0';

    if (send(sock, command, strlen(command), 0) < 0) {
        perror("Send failed");
        close(sock);
        return -1;
    }
    printf("Sent command: %s\n", command);

    ssize_t bytes_received = read(sock, buffer, BUFFER_SIZE);
    if (bytes_received < 0) {
        perror("Error reading from socket");
        close(sock);
        return -1;
    } else if (bytes_received == 0) {
        printf("Server closed connection\n");
        close(sock);
        return 0;
    }

    buffer[bytes_received] = '\0';
    printf("Received response: %s\n", buffer);

    close(sock);
    return 0;
}
