#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081
#define SERVER_ADDRESS "127.0.0.1"
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char command[BUFFER_SIZE];
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Eroare creare socket");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_ADDRESS, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    for (;;) {
        fprintf(stdout, "> ");

        if (fgets(command, sizeof(command), stdin) == NULL) {
            perror("Eroare la input");
            continue;
        }
        
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "translate") == 0) {
            send(sock, command, strlen(command), 0);
            //read(sock, buffer, BUFFER_SIZE);
            fprintf(stdout, "Text translatat cu succes!\n");
        } else {
            break;
        }
        memset(command, 0, BUFFER_SIZE);
        memset(buffer, 0, BUFFER_SIZE);
    }

    close(sock);
    return 0;
}