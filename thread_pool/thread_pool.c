#include "./thread_pool.h"

pthread_mutex_t mutex_queue;
pthread_cond_t cond_queue;

Job operations[LOAD_BOUND];
int job_count = 0;
bool shutdown = false;

int job_handle(/*Job *op*/) {
    // int host_fd = op->host_fd;
    // int client_fd = op->client_fd;
    // int kq = op->kq;

    // char buffer[PACK_LIM] = {'\0'};
    // ssize_t bytes = recv(client_fd, buffer, PACK_LIM - 1, 0);

    // // Print contents to server debug
    // printf("---- Got ----\n");
    // printf("\n%s\n", buffer);
    // printf("-------------\n");

    // if (bytes > 0) {
    //     int packet_length = 0; // Length of the packet
    //     struct packet_node *packets = response_list(buffer);
    //     if (packets != NULL) { // Send the full response in chunk
    //         while (packets != NULL) {
    //             send(client_fd, packets->packet, packets->length, 0);
                
    //             // Move onto the next packet
    //             struct packet_node *next = packets->next;
    //             free(packets->packet);
    //             free(packets);
    //             packets = next;
    //         }
    //     }
    //     else printf("\nFATAL ERROR GENERATING HOST RESPONSE.\n");
    // }

    return 0;
}

int job_enqueue(Job op) {
    int busy = false;

    pthread_mutex_lock(&mutex_queue);

    // Check if the server threads are busy
    if (job_count < LOAD_BOUND - 1) operations[job_count++] = op;
    else busy = true;
    
    pthread_mutex_unlock(&mutex_queue);
    pthread_cond_signal(&cond_queue);

    return busy;
}

void *thread_pool_start() {
    while (1) {
        Job job;

        pthread_mutex_lock(&mutex_queue);
        while (job_count == 0 && !shutdown) {
            pthread_cond_wait(&cond_queue, &mutex_queue);
        }

        if (job_count > 0) {
            job = operations[0];
            for (int i = 0; i < job_count - 1; i++) {
                operations[i] = operations[i + 1];
            }

            job_count--; 
        }
        else if (shutdown) {
            pthread_mutex_unlock(&mutex_queue);
            break;
        }

        pthread_mutex_unlock(&mutex_queue);
    
        // Communicate over the socket
        job_handle(&job);
    }

    return NULL;
}

pthread_t *thread_pool_init(int num_threads) {
    pthread_mutex_init(&mutex_queue, NULL);
    pthread_cond_init(&cond_queue, NULL);

    pthread_t *threads = (pthread_t *) malloc(sizeof(pthread_t) * num_threads);
    if (threads == NULL) {
        fprintf(stderr, "[ERROR]: Could not allocate thread pool\n");
        return NULL;
    }
    
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, thread_pool_start, NULL) != 0) {
            return NULL; 
        }
    }

    return threads;
}

int thread_pool_destroy(pthread_t *threads, int num_threads) {
    // Signal shutdown to worker threads
    pthread_mutex_lock(&mutex_queue);
    shutdown = true;
    pthread_cond_broadcast(&cond_queue);
    pthread_mutex_unlock(&mutex_queue);

    for (int i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL)) {
            return 1;
        }
    }

    free(threads);
    pthread_mutex_destroy(&mutex_queue);
    pthread_cond_destroy(&cond_queue);

    return 0;
}

