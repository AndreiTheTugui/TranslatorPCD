#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

#define SOCKET_PATH "/tmp/translator.sock" /* path-ul catre socket-ul unix */
#define BUFFER_SIZE 1024

int sock = 0;

void helpCommand() {
    fprintf(stdout, "Comenzi:\n");
    fprintf(stdout, "  add_language <limba> - Adauga o limba noua pentru traducere\n");
    fprintf(stdout, "  remove_language <limba> - Sterge o limba existenta pentru a nu traducere\n");
    fprintf(stdout, "  block_client <utilizator> - Blocheaza clientul de a mai folosi aplicatia\n");
    fprintf(stdout, "  unblock_client <utilizator> - Ofera din nou acces clientului blocat pe aplicatie\n");
    fprintf(stdout, "  show_connected_clients - Arata numarul de conexiuni active\n");
    fprintf(stdout, "  help - Eu sunt :)! Afisez comenzile disponibile\n");
    fprintf(stdout, "  exit - Iesire program\n");
}

void exitCommand(int sock) {
    send(sock, "exit", strlen("exit"), 0);
}

void sigint_handler(int sig) { /* folosim aceasta functie pentru a detecta semnalul SIGINT */
    exitCommand(sock);
    exit(EXIT_SUCCESS);
}

int main() {
    struct sockaddr_un serv_addr;
    char command[BUFFER_SIZE];
    char buffer[BUFFER_SIZE] = {0};

    signal(SIGINT, sigint_handler);

    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("Eroare creare socket");
        exit(EXIT_FAILURE);
    }

    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, SOCKET_PATH, sizeof(serv_addr.sun_path) - 1);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Eroare conexiune socket");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Bine ai venit, administrator!\n");

    for (;;) {
        fprintf(stdout, "> ");

        if (fgets(command, sizeof(command), stdin) == NULL) {
            perror("Eroare la input");
            continue;
        }
        
        command[strcspn(command, "\n")] = '\0';

        if (strncmp(command, "add_language ", strlen("add_language ")) == 0) {
            send(sock, command, strlen(command), 0);
            fprintf(stdout, "Limba a fost adaugata cu succes!\n");
        }
        else if (strncmp(command, "remove_language ", strlen("remove_language ")) == 0) {
            send(sock, command, strlen(command), 0);
            fprintf(stdout, "Limba a fost stearsa cu succes!\n");
        }
        else if (strncmp(command, "block_client ", strlen("block_client ")) == 0) {
            send(sock, command, strlen(command), 0);
            fprintf(stdout, "Clientul a fost blocat cu succes!\n");
        }
        else if (strncmp(command, "unblock_client ", strlen("unblock_client ")) == 0) {
            send(sock, command, strlen(command), 0);
            fprintf(stdout, "Clientul blocat poate folosi din nou aplicatia!\n");
        }
        else if (strcmp(command, "show_connected_clients") == 0) {
            send(sock, "show_connected_clients", strlen("show_connected_clients"), 0);
            read(sock, buffer, BUFFER_SIZE);
            fprintf(stdout, "Numar conexiuni active: %s\n", buffer);
        }
        else if (strcmp(command, "help") == 0) {
            helpCommand();
        }
        else if (strcmp(command, "exit") == 0) {
            exitCommand(sock);
            break;
        }
        memset(command, 0, BUFFER_SIZE);
        memset(buffer, 0, BUFFER_SIZE);

    }

    close(sock);
    exit(EXIT_SUCCESS);
}