#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 

#define SERV_TCP_PORT 8080
#define MAXLINE 512

int main(int argc, char *argv[])
{
    ssize_t rc;
    socklen_t clilen;
    int sockfd, newsockfd, childpid;

    struct sockaddr_in cli_addr, serv_addr;
    char msg[MAXLINE];

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror("socket error");

    // asignarea adresei si portului serverului spre care clientul poate trimite date
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(SERV_TCP_PORT);

    // conectarea la server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("error connecting to server");
        exit(-1);
    }

    // loop infinit pentru citirea comenzilor de la utilizator
    for(;;) {
        printf("Test: ");
        scanf("%s", msg);

        if (send(sockfd, msg, strlen(msg), 0) < 0)
        {
            perror("error sending message to server");
            exit(-1);
        }

        if ((rc = recv(sockfd, msg, MAXLINE, 0)) > 0)
        {
            msg[rc] = '\0';
            printf("%s\n", msg);
        }
    }
}