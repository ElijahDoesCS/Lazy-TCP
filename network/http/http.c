#include "./http.h"

char *http_serve_file(const char *filepath, int *message_len) {
    FILE *f = fopen(filepath, "rb");
    if (!f) {
        fprintf(stderr, "[ERROR]: Could not get file %s\n", filepath);
        char *error = strdup("HTTP/1.1 404 Not Found\r\n\r\n");
        *message_len = strlen(error);
        return error;
    }
    
    // Get file size
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    // Read file
    char *file_content = malloc(file_size);
    size_t read_size = fread(file_content, 1, file_size, f);
    fclose(f);
    
    if (read_size != (size_t) file_size) {
        free(file_content);
        char *error = strdup("HTTP/1.1 500 Internal Server Error\r\n\r\n");
        *message_len = strlen(error);
        return error;
    }
    
    // Build response
    char headers[256];
    int header_len = snprintf(headers, sizeof(headers),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %ld\r\n"
        "Connection: close\r\n"
        "\r\n",
        file_size);
    
    // Allocate response buffer
    *message_len = header_len + file_size;
    char *response = malloc(*message_len);
    
    memcpy(response, headers, header_len);
    memcpy(response + header_len, file_content, file_size);
    
    free(file_content);
    return response;
}

// Just going to return a response as a request, but we are going to clear the buffer as necessary
char *http_handle_request(char *data, int data_len, int *message_len) {
    // Check if it's a GET request
    if (data_len >= 4 && strncmp((char*)data, "GET ", 4) == 0) {
        return http_serve_file("./network/http/assets/Hello.html", message_len);
    }
    
    // Not a GET - return error
    char *error = strdup("HTTP/1.1 400 Bad Request\r\n\r\n");
    *message_len = strlen(error);
    return error;
}

