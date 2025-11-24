#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>

#define BUFFER_SIZE 104857


// Funzioni metodi HTTP
void get(char *buffer, int client_fd, char *file_name);
void delete(char *buffer, int client_fd, char *file_name);
void post(char *buffer, int client_fd, char *file_name);
void put(char *buffer, int client_fd, char *file_name);


// Funzioni Status Codes HTTP
/*************Successful 2xx***************/
void ok_200(int client_fd, const char *file_ext, const char *body, ssize_t body_len);
void created_201(int client_fd, const char *file_ext, const char *body, ssize_t body_len);
void no_content_204 (char *response, size_t *response_size);

/**************Client Error 4xx**************/
void bad_request_400 (char *response, size_t *response_size);
void unauthorized_401 (char *response, size_t *response_size);
void not_found_404 (char *response, size_t *response_size);

/*************Server Error 5xx***************/
void internal_server_error_500 (char *response, size_t *response_size);
void not_implemented_501 (char *response, size_t *response_size);


// Funzioni di utilità
char *get_mime_type(const char *file_ext);
char *get_file_extension(const char *file_name);
char *url_decode(const char *src);
int get_file_fd(const char *file_name, char *response, size_t *response_size);
char* get_body_data(const char *buffer);
int get_content_length(const char *buffer);

// Funzioni di autenticazione
int check_authentication(int client_fd, const char *buffer, const char *file_name, int flag);

#endif
