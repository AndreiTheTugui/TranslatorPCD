#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/translator.sock"
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_un serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, SOCKET_PATH, sizeof(serv_addr.sun_path) - 1);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    send(sock, "Hello from Unix client", strlen("Hello from Unix client"), 0);
    printf("Message sent to server\n");

    read(sock, buffer, BUFFER_SIZE);
    printf("Message from server: %s\n", buffer);

    close(sock);
    return 0;
}