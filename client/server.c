#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

<<<<<<< Updated upstream
#define SERV_TCP_PORT 8080
#define MAXLINE 512
#define MAX_CLIENTS 50


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

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("EROARE server: nu pot sa asignez un nume adresei locale");
    }

    printf("Waiting for client connections...\n");

    listen(sockfd, MAX_CLIENTS);

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
=======
#define PORT 8888

int request_handler(void *cls, struct MHD_Connection *connection, const char *url, 
                   const char *method, const char *version, const char *upload_data,
                   size_t *upload_data_size, void **con_cls) {
    if (strcmp(method, "POST") != 0) {
        return MHD_NO;
>>>>>>> Stashed changes
    }

    static int dummy;
    if (&dummy != *con_cls) {
        *con_cls = &dummy;
        return MHD_YES;
    }

    if (*upload_data_size != 0) {
        // Here we should save the upload data to a file or a buffer
        // This example just prints the uploaded data
        printf("Received data: %s\n", upload_data);
        *upload_data_size = 0;
        return MHD_YES;
    } else {
        const char *translated_text = "Translated text";

        struct MHD_Response *response = MHD_create_response_from_buffer(strlen(translated_text), 
                                                                        (void *)translated_text, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    }
}

int main() {
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL, 
                              &request_handler, NULL, MHD_OPTION_END);
    if (NULL == daemon) return 1;

    printf("Server running on port %d\n", PORT);
    getchar();

    MHD_stop_daemon(daemon);
    return 0;
}
