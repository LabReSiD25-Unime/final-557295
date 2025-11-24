#include "http_server.h"
#include <stdbool.h>
#include <dirent.h>
#include <ctype.h>

// Funzione per ottenere il tipo MIME in base all'estensione del file
char *get_mime_type(const char *file_ext) {
    if (strcmp(file_ext, "html") == 0) return "text/html";
    if (strcmp(file_ext, "css") == 0) return "text/css";
    if (strcmp(file_ext, "js") == 0) return "application/javascript";
    if (strcmp(file_ext, "png") == 0) return "image/png";
    if (strcmp(file_ext, "jpg") == 0) return "image/jpeg";
    if (strcmp(file_ext, "gif") == 0) return "image/gif";
    return "application/octet-stream";
}

// Funzione per ottenere l'estensione del file
char *get_file_extension(const char *file_name) {
    char *dot = strrchr(file_name, '.');
    if (!dot || dot == file_name) return "";
    return dot + 1;
}

// Funzione per decodificare URL
char *url_decode(const char *src){
    size_t src_len = strlen(src);
    char *decoded = malloc(src_len + 1);
    size_t decoded_len = 0;

    // Decodifica da %2x a esadecimale
    for(size_t i = 0; i < src_len; i++){
        if(src[i] == '%' && i + 2 < src_len){
            int hex_val;
            sscanf(src + i + 1, "%2x", &hex_val);
            decoded[decoded_len++] = hex_val;
            i += 2;
        }else decoded[decoded_len++] = src[i];
    }

    decoded[decoded_len] = '\0';
    return decoded;
}

// Funzione per ottenere il file descriptor del file richiesto
int get_file_fd(const char *file_name, char *response, size_t *response_size) {
    int file_fd = open(file_name, O_RDONLY);

    if(file_fd == -1){
        // Se il file non esiste, prepara la risposta 404
        not_found_404(response, response_size);
    } 
    
    return file_fd;
}

// Funzione per trovare l'inizio del corpo della richiesta HTTP
char *find_body_start(const char* buffer){
    // Cerca la sequenza di fine header HTTP
    const char* separator = "\r\n\r\n";
    char* body_start = strstr(buffer, separator);
    
    // Restituisce il puntatore all'inizio del corpo della richiesta
    if (body_start) {
        return body_start + 4;
    }
    return NULL;
}

// Funzione per ottenere il valore di Content-Length dall'header HTTP
int get_content_length(const char *buffer){
    const char *header_key = "Content-Length: ";
    char *header_line = strstr(buffer, header_key);

    if(header_line){
        const char *value_start = header_line + strlen(header_key);
        return atoi(value_start);
    }
    return -1;
}

char* get_body_data(const char *buffer){

    // Trova l'inizio del body nella request
    char *body_start = find_body_start(buffer);
    if (body_start == NULL) {
        // Request malformata
        return NULL;
    }

    // Trova la lunghezza del body
    int content_length = get_content_length(buffer);
    if (content_length == -1){
        // Request malformata
        return NULL;
    }

    // Estrae il body della request
    char *body_data = (char *) malloc(content_length + 1);
    strncpy(body_data, body_start, content_length);
    body_data[content_length] = '\0';

    return body_data;
}
