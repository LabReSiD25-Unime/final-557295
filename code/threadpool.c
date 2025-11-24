#include "threadpool.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

// Funzione eseguita da ogni thread worker
static void *worker_thread(void *arg) {
    thread_pool_t *pool = (thread_pool_t *)arg;

    while (1) {
        pthread_mutex_lock(&(pool->lock));

        // Aspetta finché non c'è un lavoro o il pool non viene chiuso
        while (pool->task_count == 0 && !pool->shutdown) {
            pthread_cond_wait(&(pool->notify), &(pool->lock));
        }

        // Se il pool è in fase di chiusura, esci
        if (pool->shutdown) {
            pthread_mutex_unlock(&(pool->lock));
            pthread_exit(NULL);
        }

        // Prendi un lavoro dalla coda
        int *client_fd = pool->queue[pool->head];
        pool->head = (pool->head + 1) % MAX_QUEUE_SIZE;
        pool->task_count--;

        pthread_mutex_unlock(&(pool->lock));

        // Esegui il lavoro
        parse_request(*client_fd);
        free(client_fd);
    }
}

// Creazione del Thread Pool
thread_pool_t *thread_pool_create() {
    // Se i parametri non sono validi, ritorna NULL
    if (MAX_THREADS <= 0 || MAX_QUEUE_SIZE <= 0) return NULL;

    thread_pool_t *pool = (thread_pool_t *)malloc(sizeof(thread_pool_t));
    if (pool == NULL) return NULL;

    pool->head = pool->tail = pool->task_count = 0;
    pool->shutdown = 0;

    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * MAX_THREADS);
    
    if (pool->threads == NULL) {
        perror("Errore nell'allocazione della memoria per il thread pool");
        free(pool->threads);
        free(pool);
        return NULL;
    }

    // Inizializza mutex e variabile di condizione
    pthread_mutex_init(&(pool->lock), NULL);
    pthread_cond_init(&(pool->notify), NULL);

    // Avvia i thread worker
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_create(&(pool->threads[i]), NULL, worker_thread, (void *)pool);
        pthread_detach(pool->threads[i]); // Li avviamo già detached
    }

    return pool;
}

// Aggiunta di un task al Thread Pool
int thread_pool_add_task(thread_pool_t *pool, int *client_fd) {
    // Lock della coda
    pthread_mutex_lock(&(pool->lock));

    if (pool->task_count == MAX_QUEUE_SIZE) {
        // Coda piena
        pthread_mutex_unlock(&(pool->lock));
        return -1;
    }

    // Aggiungi il task alla coda
    pool->queue[pool->tail] = client_fd;
    pool->tail = (pool->tail + 1) % MAX_QUEUE_SIZE;
    pool->task_count++;

    // Sveglia un worker
    pthread_mutex_unlock(&(pool->lock));
    pthread_cond_signal(&(pool->notify));

    return 0;
}

int thread_pool_destroy(thread_pool_t *pool) {
    pthread_mutex_lock(&(pool->lock));
    pool->shutdown = 1;
    pthread_cond_broadcast(&(pool->notify)); // Sveglia tutti
    pthread_mutex_unlock(&(pool->lock));

    
    free(pool->threads);
    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->notify);
    free(pool);

    return 0;
}