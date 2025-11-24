#include "http_server.h"

void get(char *buffer, int client_fd, char *file_name) {
    // Dichiara variabili risposta
    char *response = (char *) malloc(BUFFER_SIZE * sizeof(char));
    size_t response_size = 0;

    // Controlla se il client sta cercando di accedere ad una cartella protetta
    if (check_authentication(client_fd, buffer, file_name, 0) == -1){
        // Non autorizzato, invia 401 Unauthorized
        unauthorized_401(response, &response_size);
        send(client_fd, response, response_size, 0);

        free(response);
        return;
    }

    // Ottiene il file descriptor del file richiesto
    int file_fd;
    if ((file_fd = get_file_fd(file_name, response, &response_size)) == -1) {
        // Invia risposta 404 al client
        send(client_fd, response, response_size, 0);
        
        free(response);
        return;
    }

    // Copia il contenuto del file
    char *body = (char *) malloc(BUFFER_SIZE * sizeof(char));
    ssize_t bytes_read = read(file_fd, body, BUFFER_SIZE);
    if (bytes_read < 0) bytes_read = 0;
    response_size = bytes_read;


    // Elabora risposta 200 OK
    char *file_ext = get_file_extension(file_name);
    ok_200(client_fd, file_ext, body, bytes_read);
    close(file_fd);

    free(body);
    free(response);
}

void delete(char *buffer, int client_fd, char *file_name) {
    // Dichiara variabili risposta
    char *response = (char *) malloc(BUFFER_SIZE * 2 * sizeof(char));
    size_t response_size = 0;

    // Controlla i permessi di autenticazione
    if (check_authentication(client_fd, buffer, file_name, 1) == -1){
        // Non autorizzato, invia 401 Unauthorized
        unauthorized_401(response, &response_size);
        send(client_fd, response, response_size, 0);

        free(response);
        return;
    }

    // Controlla se il file esiste
    if(get_file_fd(file_name, response, &response_size) == -1) {
        // Invia risposta 404 al client
        send(client_fd, response, response_size, 0);
        
        free(response);
        return;
    }

    // Prova a eliminare il file
    if (remove(file_name) != 0)
        // Se fallisce, elabora risposta 500 Internal Server Error
        internal_server_error_500(response, &response_size);
    else{
        // Elabora risposta 204 No Content
        no_content_204(response, &response_size);
    }
    send(client_fd, response, response_size, 0);

    free(response);
}

void post(char *buffer, int client_fd, char *file_name) {
    // Dichiara variabili risposta
    char *response = (char *) malloc(BUFFER_SIZE * sizeof(char));    size_t response_size = 0;

    if (check_authentication(client_fd, buffer, file_name, 1) == -1){
        // Non autorizzato, invia 401 Unauthorized
        unauthorized_401(response, &response_size);
        send(client_fd, response, response_size, 0);

        free(response);
        return;
    }

    // Estrae il body della request
    char *body_data;
    if((body_data = get_body_data(buffer)) == NULL){
        // Request malformata, invia 400 Bad Request
        bad_request_400(response, &response_size);
        send(client_fd, response, response_size, 0);

        free(response);
        return;
    }

    // Ottiene il file descriptor del file richiesto
    int file_fd = open(file_name, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (file_fd == -1){
        // Errore nell'aprire/creare il file
        internal_server_error_500(response, &response_size);
        send(client_fd, response, response_size, 0);
    
        free(body_data);
        free(response);
        return;
    }

    // Scrive / Appendo il body nel file
    ssize_t content_length = get_content_length(buffer);
    if(write(file_fd, body_data, content_length) == -1) internal_server_error_500(response, &response_size);
    else {
        char *file_ext = get_file_extension(file_name);
        ok_200(client_fd, file_ext, body_data, content_length);

        close(file_fd);
        free(body_data);
        free(response);
        return;
    }

    close(file_fd);
    send(client_fd, response, response_size, 0);

    free(body_data);
    free(response);
}

void put(char *buffer, int client_fd, char *file_name){
    // Dichiara variabili risposta
    char *response = (char *) malloc(BUFFER_SIZE * sizeof(char));    size_t response_size = 0;

    // Controlla i permessi di autenticazione
    if (check_authentication(client_fd, buffer, file_name, 1) == -1){
        // Non autorizzato, invia 401 Unauthorized
        unauthorized_401(response, &response_size);
        send(client_fd, response, response_size, 0);

        free(response);
        return;
    }

    // Estrae il body della request
    char *body_data;
    if((body_data = get_body_data(buffer)) == NULL){
        // Request malformata, invia 400 Bad Request
        bad_request_400(response, &response_size);
        send(client_fd, response, response_size, 0);

        free(response);
        return;
    }

    // Controlla se il file esiste già
    int existing_file_fd = get_file_fd(file_name, response, &response_size);
    int flag_different = 0;

    if (existing_file_fd != -1){
        // Il file esiste, controlla dimensioni del file
        struct stat file_stat;
        fstat(existing_file_fd, &file_stat);
        int content_length = get_content_length(buffer);

        if (file_stat.st_size == content_length){
            char *file_ext_data = (char *) malloc(content_length);
            read(existing_file_fd, file_ext_data, content_length);

            // Confronta il contenuto del file con il body della request
            if (memcmp(file_ext_data, body_data, content_length) == 0) flag_different = 1; // I file sono identici
            free(file_ext_data);
        }
        close(existing_file_fd);
    }

    if (flag_different){
        // I file sono identici, invia 204 No Content
        no_content_204(response, &response_size);
        send(client_fd, response, response_size, 0);

        free(body_data);
        free(response);
        return;
    }

    int file_fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_fd == -1){
        // Errore nell'aprire/creare il file
        internal_server_error_500(response, &response_size);
        send(client_fd, response, response_size, 0);

        free(body_data);
        free(response);
        return;
    }

    // Scrive il body nel file
    int content_length = get_content_length(buffer);
    if(write(file_fd, body_data, content_length) == -1) internal_server_error_500(response, &response_size);
    else {
        char *file_ext = get_file_extension(file_name);
        created_201(client_fd, file_ext, body_data, content_length);

        close(file_fd);
        free(body_data);
        return;
    }

    close(file_fd);
    send(client_fd, response, response_size, 0);

    free(body_data);
    return;
}