#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PORT 8888

int request_handler(void *cls, struct MHD_Connection *connection, const char *url, 
                   const char *method, const char *version, const char *upload_data,
                   size_t *upload_data_size, void **con_cls) {
    if (strcmp(method, "POST") != 0) {
        return MHD_NO;
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
