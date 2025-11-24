#include "http_server.h"
#include <regex.h>
#include <sys/time.h> 

void parse_request(int client_fd) {
    // Imposta un Timeout di 5 secondi
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    char *buffer = (char *) malloc(BUFFER_SIZE * sizeof(char));
    if (!buffer) {
        perror("Malloc buffer fallito");
        close(client_fd);
        return;
    }

    int keep_alive = 1; // HTTP 1.1 è keep-alive di default

    // Loop di connessione persistente
    while (keep_alive) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

        if (bytes_received <= 0) {
            // Se <= 0, il client ha chiuso o c'è stato un timeout/errore
            if (bytes_received == 0) printf("Client fd %d disconnesso.\n", client_fd);
            else perror("recv fallito (o timeout)");
            break;
        }

        // Controlla se il client vuole chiudere (Connection: close)
        if (strstr(buffer, "Connection: close") != NULL) {
            keep_alive = 0;
        }

        // Parsing della richiesta HTTP
        regex_t regex;
        regcomp(&regex, "([A-Z]+) /([^ ]*)", REG_EXTENDED);
        regmatch_t matches[3];

        if (regexec(&regex, buffer, 3, matches, 0) == 0) {
            // Estrae il metodo
            size_t method_len = matches[1].rm_eo - matches[1].rm_so;
            char method[method_len + 1];
            strncpy(method, buffer + matches[1].rm_so, method_len);
            method[method_len] = '\0';

            // Estrae il nome del file
            size_t path_len = matches[2].rm_eo - matches[2].rm_so;
            char url_encoded_file_name[path_len + 1];
            strncpy(url_encoded_file_name, buffer + matches[2].rm_so, path_len);
            url_encoded_file_name[path_len] = '\0';
            char *file_name = url_decode(url_encoded_file_name);

            // Dispatch
            if (strcmp(method, "GET") == 0) get(buffer, client_fd, file_name);
            else if (strcmp(method, "POST") == 0) post(buffer, client_fd, file_name);
            else if (strcmp(method, "PUT") == 0) put(buffer, client_fd, file_name);
            else if (strcmp(method, "DELETE") == 0) delete(buffer, client_fd, file_name);
            else {
                // Metodo non implementato
                char response[BUFFER_SIZE];
                size_t response_size;
                not_implemented_501(response, &response_size);
                send(client_fd, response, response_size, 0);
            }
            free(file_name);
        } else {
            // Richiesta malformata
            char response[BUFFER_SIZE];
            size_t response_size;
            bad_request_400(response, &response_size);
            send(client_fd, response, response_size, 0);
        }
        
        regfree(&regex);

        // Se il client ha chiesto di chiudere usciamo dopo la risposta
        if (!keep_alive) {
            break;
        }
        
    }

    printf("Chiusura connessione per fd %d\n", client_fd);
    close(client_fd);
    free(buffer);
    return;
}
