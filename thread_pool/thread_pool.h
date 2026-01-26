#ifndef THREAD_POOL_H
#define THREAD_POOL_H

typedef struct Job Job;

struct Job {
    int host_fd;
    int client_fd;
    int kq;
};

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>

# define WORKER_THREAD_COUNT 16
# define LOAD_BOUND 1280 

int job_handle(/*Job *op*/);
int job_enqueue(Job op);

void *thread_pool_start();
pthread_t *thread_pool_init(int num_threads);
int thread_pool_destroy(pthread_t *threads, int num_threads);

#endif
