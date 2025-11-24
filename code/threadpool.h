#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <stddef.h>

#define MAX_THREADS 4
#define MAX_QUEUE_SIZE 256

// Struttura del Thread Pool
typedef struct {
    pthread_t *threads;         // Array di thread
    int *queue[MAX_QUEUE_SIZE]; // Coda dei lavori 
    int head;                   // Testa della coda
    int tail;                   // Coda della coda
    int task_count;             // Numero di lavori in coda
    pthread_mutex_t lock;       // Mutex per proteggere la coda
    pthread_cond_t notify;      // Variabile di condizione per svegliare i thread
    int shutdown;               // Flag per terminare il pool
} thread_pool_t;

// Creazione del Thread Pool
thread_pool_t *thread_pool_create();

// Aggiunta di un task al Thread Pool
int thread_pool_add_task(thread_pool_t *pool, int *client_fd);

// Distruzione del Thread Pool
int thread_pool_destroy(thread_pool_t *pool);

// Funzione thread pool
void parse_request(int client_fd);
#endif 