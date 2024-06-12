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

int main() {
    Admin admin;
    admin.num_languages = 0;
    admin.max_clients = 10;
    memset(admin.blocked_clients, 0, sizeof(admin.blocked_clients));

    add_language(&admin, "en");
    add_language(&admin, "ro");
    add_language(&admin, "de");

    block_client(&admin, 1);
    set_max_clients(&admin, 20);

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

