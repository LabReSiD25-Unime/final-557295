#include "http_server.h"

/*************Successful 2xx***************/
void ok_200(int client_fd, const char *file_ext, const char *body, ssize_t body_len) {
    char header[BUFFER_SIZE];
    const char *mime_type = get_mime_type(file_ext);
    int header_len = snprintf(header, BUFFER_SIZE,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "\r\n",
        mime_type, body_len);

    send(client_fd, header, header_len, 0);
    send(client_fd, body, body_len, 0);
}


void created_201(int client_fd, const char *file_ext, const char *body, ssize_t body_len) {
    char header[BUFFER_SIZE];
    const char *mime_type = get_mime_type(file_ext);
    int header_len = snprintf(header, BUFFER_SIZE,
        "HTTP/1.1 201 Created\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "\r\n",
        mime_type, body_len);

    send(client_fd, header, header_len, 0);
    send(client_fd, body, body_len, 0);
}


void no_content_204 (char *response, size_t *response_size){
    snprintf(response, BUFFER_SIZE, 
        "HTTP/1.1 204 No Content\r\n");
    *response_size = strlen(response);
}


/**************Client Error 4xx**************/
void bad_request_400 (char *response, size_t *response_size){
    snprintf(response, BUFFER_SIZE, 
        "HTTP/1.1 400 Bad Request\r\n"
        "\r\n");
    *response_size = strlen(response);
}

void unauthorized_401 (char *response, size_t *response_size){
    snprintf(response, BUFFER_SIZE, 
        "HTTP/1.1 401 Unauthorized\r\n"
        "\r\n");
        
    *response_size = strlen(response);
}

void not_found_404 (char *response, size_t *response_size){
    snprintf(response, BUFFER_SIZE, 
        "HTTP/1.1 404 Not Found\r\n"
        "\r\n");
        
    *response_size = strlen(response);
}

/*************Server Error 5xx***************/
void internal_server_error_500 (char *response, size_t *response_size){
    snprintf(response, BUFFER_SIZE, 
        "HTTP/1.1 500 Internal Server Error\r\n"
        "\r\n");
        
    *response_size = strlen(response);
}

void not_implemented_501 (char *response, size_t *response_size){
    snprintf(response, BUFFER_SIZE, 
        "HTTP/1.1 501 Not Implemented\r\n"
        "\r\n");
        
    *response_size = strlen(response);
}