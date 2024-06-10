#include "soapH.h"
#include "service.nsmap"

int ns__upload(struct soap *soap, char *fileName, char *fileContent, char **response) {
    char path[256];
    snprintf(path, sizeof(path), "uploads/%s", fileName);
    FILE *file = fopen(path, "wb");
    if (file) {
        size_t length = soap->blist->size;
        fwrite(fileContent, 1, length, file);
        fclose(file);
        *response = "File uploaded successfully.";
        return SOAP_OK;
    } else {
        return soap_sender_fault(soap, "File upload failed", "Cannot open file");
    }
}

int ns__download(struct soap *soap, char *fileName, char **fileContent) {
    char path[256];
    snprintf(path, sizeof(path), "uploads/%s", fileName);
    FILE *file = fopen(path, "rb");
    if (file) {
        fseek(file, 0, SEEK_END);
        size_t length = ftell(file);
        fseek(file, 0, SEEK_SET);
        *fileContent = (char*)soap_malloc(soap, length);
        fread(*fileContent, 1, length, file);
        fclose(file);
        return SOAP_OK;
    } else {
        return soap_sender_fault(soap, "File download failed", "Cannot open file");
    }
}

int main() {
    struct soap soap;
    soap_init(&soap);
    if (soap_bind(&soap, NULL, 8080, 100) < 0)
        soap_print_fault(&soap, stderr);
    else {
        while (soap_accept(&soap) >= 0) {
            soap_serve(&soap);
            soap_end(&soap);
        }
    }
    soap_done(&soap);
    return 0;
}
