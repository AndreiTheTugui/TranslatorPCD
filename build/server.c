#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/un.h>
#include <microhttpd.h>
#include <sys/stat.h>
#include <time.h>

#define SOCKET_PORT 8080
#define BUFFER_SIZE 1024
#define UNIX_SOCKET_PATH "/tmp/translator.sock"
#define HTTP_PORT 8888
#define LANGUAGES_FILE "/tmp/languages.txt"

int active_clients = 0;
const int max_clients = 10;

void* handle_client(void* client_socket) {
    int socket = *(int*)client_socket;
    free(client_socket);

    char buffer[BUFFER_SIZE];
    read(socket, buffer, BUFFER_SIZE);
    printf("Received from TCP client: %s\n", buffer);
    send(socket, "Hello from server", strlen("Hello from server"), 0);

    close(socket);
    return NULL;
}

void start_tcp_server() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(SOCKET_PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, max_clients) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        active_clients++;
        int* client_socket = malloc(sizeof(int));
        *client_socket = new_socket;
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, client_socket);
        pthread_detach(thread_id);
    }
}

void* handle_unix_client(void* client_socket) {
    int socket = *(int*)client_socket;
    free(client_socket);

    char buffer[BUFFER_SIZE];
    char send_buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = read(socket, buffer, BUFFER_SIZE);
        if (bytes_read <= 0) {
            break;
        }
        printf("Unix socket received: %s\n", buffer);
        if (strncmp(buffer, "add_language", strlen("add_language")) == 0) {
            FILE *languages_file = fopen(LANGUAGES_FILE, "a");
            if (languages_file == NULL) {
                perror("Could not open languages file for writing");
                return NULL;
            }

            char* language = buffer + strlen("add_language") + 1;
            fprintf(languages_file, "%s\n", language);

            fclose(languages_file);
        }
        else if (strncmp(buffer, "remove_language", strlen("remove_language")) == 0) {
            FILE *languages_file = fopen(LANGUAGES_FILE, "r");
            if (languages_file == NULL) {
                perror("Could not open languages file for reading");
                return NULL;
            }

            FILE *temp_file = fopen("/tmp/languages_temp.txt", "w");
            if (temp_file == NULL) {
                perror("Could not open temporary file for writing");
                fclose(languages_file);
                return NULL;
            }

            char line[BUFFER_SIZE];
            char* language_to_remove = buffer + strlen("remove_language") + 1;
            while (fgets(line, sizeof(line), languages_file)) {
                if (strstr(line, language_to_remove) == NULL) {
                    fputs(line, temp_file);
                }
            }

            fclose(languages_file);
            fclose(temp_file);

            remove(LANGUAGES_FILE);
            rename("/tmp/languages_temp.txt", LANGUAGES_FILE);
        }
        else if (strcmp(buffer, "show_connected_clients") == 0) {
            snprintf(send_buffer, sizeof(send_buffer), "%d", active_clients);
            send(socket, send_buffer, strlen(send_buffer), 0);
        } else if (strcmp(buffer, "exit") == 0) {
            active_clients--;
            break;
        }
    }

    return NULL;
}

void start_unix_server() {
    int unix_fd, new_socket;
    struct sockaddr_un address;
    int addrlen = sizeof(address);

    if ((unix_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == 0) {
        perror("unix socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&address, 0, sizeof(struct sockaddr_un));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, UNIX_SOCKET_PATH, sizeof(address.sun_path) - 1);

    unlink(UNIX_SOCKET_PATH);

    if (bind(unix_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(unix_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(unix_fd, 3) < 0) {
        perror("listen");
        close(unix_fd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        if ((new_socket = accept(unix_fd, NULL, NULL)) < 0) {
            perror("accept");
            close(unix_fd);
            exit(EXIT_FAILURE);
        }
        active_clients++;
        int* client_socket = malloc(sizeof(int));
        *client_socket = new_socket;
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_unix_client, client_socket);
        pthread_detach(thread_id);
    }
}

enum MHD_Result request_handler(void *cls, struct MHD_Connection *connection, const char *url, 
                   const char *method, const char *version, const char *upload_data,
                   size_t *upload_data_size, void **con_cls) {
    static char buffer[BUFFER_SIZE];
    static size_t buffer_pos = 0;
    static FILE *uploaded_file = NULL;
    static char filename[BUFFER_SIZE];

    if (strcmp(method, "POST") != 0) {
        return MHD_NO;
    }

    if (*con_cls == NULL) {
        *con_cls = buffer;
        buffer_pos = 0;
        sprintf(filename, "/tmp/uploaded_file_%ld", time(NULL));
        printf("Creating file: %s\n", filename);
        uploaded_file = fopen(filename, "wb");
        if (uploaded_file == NULL) {
            perror("fopen");
            return MHD_NO;
        }
        return MHD_YES;
    }

    if (*upload_data_size != 0) {
        printf("Writing data to file: %s\n", filename);
        fwrite(upload_data, 1, *upload_data_size, uploaded_file);
        *upload_data_size = 0;
        return MHD_YES;
    } else {
        fclose(uploaded_file);
        printf("Finished writing to file: %s\n", filename);

        struct stat file_stat;
        if (stat(filename, &file_stat) == -1) {
            perror("stat");
            return MHD_NO;
        }

        char date_buf[BUFFER_SIZE];
        strftime(date_buf, BUFFER_SIZE, "%Y-%m-%d %H:%M:%S", localtime(&file_stat.st_mtime));
        printf("File modification date and time: %s\n", date_buf);

        FILE *file = fopen(filename, "r");
        if (file == NULL) {
            perror("fopen");
            return MHD_NO;
        }

        printf("File content:\n");
        while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
            printf("%s", buffer);
        }
        fclose(file);

        const char *response_text = "File uploaded and read successfully";
        struct MHD_Response *response = MHD_create_response_from_buffer(strlen(response_text), 
                                                                        (void *)response_text, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    }
}

void* start_http_server(void* arg) {
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, HTTP_PORT, NULL, NULL, 
                              &request_handler, NULL, MHD_OPTION_END);
    if (NULL == daemon) return NULL;

    printf("HTTP Server running on port %d\n", HTTP_PORT);
    getchar();

    MHD_stop_daemon(daemon);
    return NULL;
}

int main() {
    pthread_t tcp_thread, unix_thread, http_thread;

    pthread_create(&tcp_thread, NULL, (void* (*)(void*))start_tcp_server, NULL);
    pthread_create(&unix_thread, NULL, (void* (*)(void*))start_unix_server, NULL);
    pthread_create(&http_thread, NULL, start_http_server, NULL);

    pthread_join(tcp_thread, NULL);
    pthread_join(unix_thread, NULL);
    pthread_join(http_thread, NULL);

    return 0;
}

