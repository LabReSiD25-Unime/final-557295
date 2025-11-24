#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>

#include "threadpool.h" 

#define PORT 8080
#define MAX_EVENTS 64


int main() {
    int server_fd, epoll_fd;
    struct sockaddr_in serv_addr;
    struct epoll_event ev, events[MAX_EVENTS];

    // Inizializzazione Thread Pool
    thread_pool_t *pool = thread_pool_create();
    if (pool == NULL) {
        perror("Creazione thread pool fallita");
        exit(EXIT_FAILURE);
    }

    // Inizializzazione Server Socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Creazione socket fallita"); exit(EXIT_FAILURE);
    }
    
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        perror("Bind fallito"); 
        close(server_fd); 
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, SOMAXCONN) < 0) { // Massimo size della coda di connessioni in attesa
        perror("Listen fallito"); 
        close(server_fd); 
        exit(EXIT_FAILURE);
    }

    // Inizializzazione Epoll
    if ((epoll_fd = epoll_create1(0)) < 0) {
        perror("Epoll create fallito"); 
        close(server_fd); 
        exit(EXIT_FAILURE);
    }

    // Aggiungi il server_fd a epoll
    ev.events = EPOLLIN; 
    ev.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) < 0) {
        perror("Epoll ctl (server_fd) fallito");
        close(server_fd); 
        close(epoll_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server in ascolto sulla porta %d \n", PORT);

    // Loop degli eventi
    while (1) {
        int n_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n_events < 0) {
            perror("Epoll wait fallito");
            continue;
        }

        for (int i = 0; i < n_events; i++) {
            if (events[i].data.fd == server_fd) {
                // Nuova connessione in arrivo
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                
                int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
                if (client_fd < 0) {
                    perror("Accept fallito");
                    continue;
                }

                printf("Nuova connessione accettata: fd %d\n", client_fd);

                // Aggiungiamo il nuovo client a epoll
                ev.events = EPOLLIN; // Monitora solo la lettura
                ev.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) < 0) {
                    perror("Epoll ctl (client_fd) fallito");
                    close(client_fd);
                }

            } else {
                // Dati pronti da leggere su un client esistente
                int client_fd = events[i].data.fd;
                // Per evitare che questo evento venga processato più volte lo rimuoviamo da epoll.
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);

                // Allochiamo memoria per passare il client_fd al worker
                int *client_fd_ptr = malloc(sizeof(int));
                if (client_fd_ptr == NULL) {
                    perror("malloc per client_fd_ptr fallito");
                    close(client_fd); 
                    continue;         
                }
                *client_fd_ptr = client_fd;

                // Aggiungiamo il lavoro al pool.
                thread_pool_add_task(pool, client_fd_ptr);
            }
        }
    }

    close(server_fd);
    close(epoll_fd);
    thread_pool_destroy(pool);
    return 0;
}
