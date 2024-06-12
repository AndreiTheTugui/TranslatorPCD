#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LANGUAGES 10
#define BUFFER_SIZE 256

typedef struct {
    char languages[MAX_LANGUAGES][BUFFER_SIZE];
    int blocked_clients[MAX_LANGUAGES];
    int num_languages;
    int max_clients;
} Admin;

void add_language(Admin *admin, const char *language);
void remove_language(Admin *admin, const char *language);
void block_client(Admin *admin, int client_id);
void unblock_client(Admin *admin, int client_id);
void set_max_clients(Admin *admin, int max_clients);
void display_menu();

int main() {
    Admin admin;
    admin.num_languages = 0;
    admin.max_clients = 10;
    memset(admin.blocked_clients, 0, sizeof(admin.blocked_clients));

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
        } else if (strcmp(command, "exit") == 0) {
            break;
        } else {
            printf("Invalid command\n");
        }
    }

    return 0;
}

void add_language(Admin *admin, const char *language) {
    if (admin->num_languages < MAX_LANGUAGES) {
        strcpy(admin->languages[admin->num_languages], language);
        admin->num_languages++;
        printf("Language %s added\n", language);
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
            return;
        }
    }
    printf("Language %s not found\n", language);
}

void block_client(Admin *admin, int client_id) {
    if (client_id < MAX_LANGUAGES) {
        admin->blocked_clients[client_id] = 1;
        printf("Client %d blocked\n", client_id);
    } else {
        printf("Invalid client ID\n");
    }
}

void unblock_client(Admin *admin, int client_id) {
    if (client_id < MAX_LANGUAGES) {
        admin->blocked_clients[client_id] = 0;
        printf("Client %d unblocked\n", client_id);
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
    printf("  exit - Exit the program\n");
}

