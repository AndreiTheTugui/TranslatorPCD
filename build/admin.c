#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

#define SOCKET_PATH "/tmp/translator.sock" /* path-ul catre socket-ul unix */
#define LANGUAGES_FILE "/tmp/languages.txt" /* path-ul catre fisierul cu limbi */
#define BUFFER_SIZE 1024

int sock = 0;

void helpCommand() {
    fprintf(stdout, "Comenzi:\n");
    fprintf(stdout, "  add_language <limba> - Adauga o limba noua pentru traducere\n");
    fprintf(stdout, "  remove_language <limba> - Sterge o limba existenta pentru a nu mai fi tradusa\n");
    fprintf(stdout, "  block_client - Blocheaza clientul de a mai folosi aplicatia\n");
    fprintf(stdout, "  unblock_client - Ofera din nou acces clientului blocat pe aplicatie\n");
    fprintf(stdout, "  show_connected_clients - Arata numarul de conexiuni active\n");
    fprintf(stdout, "  show_languages - Arata limbile disponibile pentru traducere\n");
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

void addLanguage(const char* language) {
    FILE *file = fopen(LANGUAGES_FILE, "a");
    if (file == NULL) {
        perror("Nu am putut deschide fisierul pentru scriere");
        return;
    }
    fprintf(file, "%s\n", language);
    fclose(file);
    fprintf(stdout, "Limba a fost adaugata cu succes!\n");
}

void removeLanguage(const char* language) {
    FILE *file = fopen(LANGUAGES_FILE, "r");
    if (file == NULL) {
        perror("Nu am putut deschide fisierul pentru citire");
        return;
    }

    FILE *tempFile = fopen("/tmp/languages_temp.txt", "w");
    if (tempFile == NULL) {
        perror("Nu am putut deschide fisierul temporar pentru scriere");
        fclose(file);
        return;
    }

    char line[BUFFER_SIZE];
    int found = 0;
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, language) == NULL) {
            fputs(line, tempFile);
        } else {
            found = 1;
        }
    }

    fclose(file);
    fclose(tempFile);

    if (found) {
        remove(LANGUAGES_FILE);
        rename("/tmp/languages_temp.txt", LANGUAGES_FILE);
        fprintf(stdout, "Limba a fost stearsa cu succes!\n");
    } else {
        remove("/tmp/languages_temp.txt");
        fprintf(stdout, "Limba nu a fost gasita!\n");
    }
}

void showLanguages() {
    FILE *file = fopen(LANGUAGES_FILE, "r");
    if (file == NULL) {
        perror("Nu am putut deschide fisierul pentru citire");
        return;
    }

    char line[BUFFER_SIZE];
    fprintf(stdout, "Limbile disponibile pentru traducere sunt:\n");
    while (fgets(line, sizeof(line), file)) {
        fprintf(stdout, "%s", line);
    }

    fclose(file);
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
            char *language = command + strlen("add_language ");
            addLanguage(language);
            // Send command to server if needed
            send(sock, command, strlen(command), 0);
        } else if (strncmp(command, "remove_language ", strlen("remove_language ")) == 0) {
            char *language = command + strlen("remove_language ");
            removeLanguage(language);
            // Send command to server if needed
            send(sock, command, strlen(command), 0);
        } else if (strcmp(command, "show_connected_clients") == 0) {
            send(sock, "show_connected_clients", strlen("show_connected_clients"), 0);
            read(sock, buffer, BUFFER_SIZE);
            fprintf(stdout, "Numar conexiuni active: %s\n", buffer);
        } else if (strcmp(command, "show_languages") == 0) {
            showLanguages();
        } else if (strcmp(command, "help") == 0) {
            helpCommand();
        } else if (strcmp(command, "exit") == 0) {
            exitCommand(sock);
            break;
        } else {
            fprintf(stdout, "Comanda necunoscuta. Tastati 'help' pentru a vedea comenzile disponibile.\n");
        }
        memset(command, 0, BUFFER_SIZE);
        memset(buffer, 0, BUFFER_SIZE);
    }

    close(sock);
    exit(EXIT_SUCCESS);
}

