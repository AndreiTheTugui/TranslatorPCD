#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/un.h>  

#define MAX_LANGUAGES 10
#define BUFFER_SIZE 256
#define MAX_CLIENTS 100

#define LANGUAGES_FILE "languages.txt"
#define BLOCKED_USERS_FILE "blocked_users.txt"
#define SERVER_IP "127.0.0.1"
#define PORT 12346
#define UNIX_SOCKET_PATH "/tmp/translator.sock"

typedef struct {
    char languages[MAX_LANGUAGES][BUFFER_SIZE];
    int blocked_clients[MAX_CLIENTS];
    int num_languages;
    int max_clients;
} Admin;

int connect_unix_socket() {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }

    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, UNIX_SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        close(sock);
        return -1;
    }

    return sock;
}

void load_languages(Admin *admin);
void save_languages(Admin *admin);
void load_blocked_clients(Admin *admin);
void save_blocked_clients(Admin *admin);
void add_language(Admin *admin, const char *language);
void remove_language(Admin *admin, const char *language);
void block_client(Admin *admin, int client_id);
void unblock_client(Admin *admin, int client_id);
void set_max_clients(Admin *admin, int max_clients);
void display_menu();
void show_connected_clients(int sock);

int main() {
    Admin admin;
    admin.num_languages = 0;
    admin.max_clients = MAX_CLIENTS;
    memset(admin.blocked_clients, 0, sizeof(admin.blocked_clients));

    load_languages(&admin);
    load_blocked_clients(&admin);

    int unix_sock = connect_unix_socket();
    if (unix_sock < 0) {
        fprintf(stderr, "Failed to connect to Unix socket server\n");
        return 1;
    }

    char command[BUFFER_SIZE];
    char language[BUFFER_SIZE];
    int client_id, max_clients;

    while (1) {
        display_menu();
        printf("Enter command: ");
        scanf("%s", command);

        if (strcmp(command, "add_language") == 0) {
            printf("Enter language: ");
            scanf("%s", language);
            add_language(&admin, language);
        } else if (strcmp(command, "remove_language") == 0) {
            printf("Enter language: ");
            scanf("%s", language);
            remove_language(&admin, language);
        } else if (strcmp(command, "block_client") == 0) {
            printf("Enter client ID: ");
            scanf("%d", &client_id);
            block_client(&admin, client_id);
        } else if (strcmp(command, "unblock_client") == 0) {
            printf("Enter client ID: ");
            scanf("%d", &client_id);
            unblock_client(&admin, client_id);
        } else if (strcmp(command, "set_max_clients") == 0) {
            printf("Enter max clients: ");
            scanf("%d", &max_clients);
            set_max_clients(&admin, max_clients);
        } else if (strcmp(command, "show_connected_clients") == 0) {
            show_connected_clients(unix_sock);
        } else if (strcmp(command, "exit") == 0) {
            save_languages(&admin);
            save_blocked_clients(&admin);
            break;
        } else {
            printf("Invalid command\n");
        }
    }

    close(unix_sock);
    return 0;
}

void load_languages(Admin *admin) {
    FILE *file = fopen(LANGUAGES_FILE, "r");
    if (file == NULL) {
        return; 
    }

    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), file) && admin->num_languages < MAX_LANGUAGES) {
        line[strcspn(line, "\n")] = 0; 
        strcpy(admin->languages[admin->num_languages], line);
        admin->num_languages++;
    }

    fclose(file);
}

void save_languages(Admin *admin) {
    FILE *file = fopen(LANGUAGES_FILE, "w");
    if (file == NULL) {
        perror("Could not open languages file for writing");
        return;
    }

    for (int i = 0; i < admin->num_languages; i++) {
        fprintf(file, "%s\n", admin->languages[i]);
    }

    fclose(file);
}

void load_blocked_clients(Admin *admin) {
    FILE *file = fopen(BLOCKED_USERS_FILE, "r");
    if (file == NULL) {
        return; 
    }

    int client_id;
    while (fscanf(file, "%d", &client_id) == 1) {
        if (client_id < MAX_CLIENTS) {
            admin->blocked_clients[client_id] = 1;
        }
    }

    fclose(file);
}

void save_blocked_clients(Admin *admin) {
    FILE *file = fopen(BLOCKED_USERS_FILE, "w");
    if (file == NULL) {
        perror("Error opening blocked users file for writing");
        return;
    }

    for (int i = 0; i < admin->max_clients; i++) {
        if (admin->blocked_clients[i]) {
            if (fprintf(file, "%d\n", i) < 0) {
                perror("Error writing to blocked users file");
                fclose(file); 
                return;
            }
        }
    }

    fclose(file);
}

void add_language(Admin *admin, const char *language) {
    for (int i = 0; i < admin->num_languages; i++) {
        if (strcmp(admin->languages[i], language) == 0) {
            printf("Language %s already exists\n", language);
            return;
        }
    }

    if (admin->num_languages < MAX_LANGUAGES) {
        strcpy(admin->languages[admin->num_languages], language);
        admin->num_languages++;
        printf("Language %s added\n", language);
        save_languages(admin);
    } else {
        printf("Language list is full\n");
    }
}

void remove_language(Admin *admin, const char *language) {
    for (int i = 0; i < admin->num_languages; i++) {
        if (strcmp(admin->languages[i], language) == 0) {
            for (int j = i; j < admin->num_languages - 1; j++) {
                strcpy(admin->languages[j], admin->languages[j + 1]);
            }
            admin->num_languages--;
            printf("Language %s removed\n", language);
            save_languages(admin);
            return;
        }
    }
    printf("Language %s not found\n", language);
}

void block_client(Admin *admin, int client_id) {
    if (client_id < admin->max_clients) {
        admin->blocked_clients[client_id] = 1;
        printf("Client %d blocked\n", client_id);
        save_blocked_clients(admin);
    } else {
        printf("Invalid client ID\n");
    }
}

void unblock_client(Admin *admin, int client_id) {
    if (client_id < admin->max_clients) {
        admin->blocked_clients[client_id] = 0;
        printf("Client %d unblocked\n", client_id);
        save_blocked_clients(admin);
    } else {
        printf("Invalid client ID\n");
    }
}

void set_max_clients(Admin *admin, int max_clients) {
    admin->max_clients = max_clients;
    printf("Max clients set to %d\n", max_clients);
}

void display_menu() {
    printf("\nAvailable commands:\n");
    printf("  add_language - Add a new language\n");
    printf("  remove_language - Remove a language\n");
    printf("  block_client - Block a client\n");
    printf("  unblock_client - Unblock a client\n");
    printf("  set_max_clients - Set the maximum number of clients\n");
    printf("  show_connected_clients - Show the number of connected clients\n");
    printf("  exit - Exit the program\n");
}

void show_connected_clients(int sock) {
    char buffer[BUFFER_SIZE];
    strcpy(buffer, "show_connected_clients");

    if (send(sock, buffer, strlen(buffer) + 1, 0) < 0) {
        perror("Send failed");
        return;
    }

    if (recv(sock, buffer, BUFFER_SIZE, 0) < 0) {
        perror("Receive failed");
        return;
    }

    printf("Number of connected clients: %s\n", buffer);
}

