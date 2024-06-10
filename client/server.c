#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <resolv.h> 
#include <time.h> 

#define SERV_TCP_PORT 8080
#define MAXLINE 512


int main(int argc, char *argv[])
{
    int sockfd, newsockfd, childpid;
    ssize_t rc;
    socklen_t clilen;
    struct sockaddr_in cli_addr, serv_addr;
    char msg[MAXLINE];

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror("socket error");

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(SERV_TCP_PORT);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        perror("EROARE server: nu pot sa asignez un nume adresei locale");
    printf("Waiting for client connections...\n");

    listen(sockfd, 10);

    // loop infinit pentru asteptarea conexiunii cu un client si crearea unui proces copil pentru fiecare client
    for (;;)
    {
        bzero((char *)&cli_addr, sizeof(cli_addr));
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
        {
            perror("EROARE server: accept() esuat");
            exit(-1);
        }

        if ((childpid = fork()) < 0)
        {
            perror("EROARE server: fork() esuat");
            exit(1);
        }
        else if (childpid == 0)
        {
            close(sockfd);
            printf("Client PID= %ld conectat\n", (long)getpid());
        }
        else
        {
            close(newsockfd);
            wait(NULL);
        }
    }
}