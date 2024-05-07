/**
 * critical_concurrency
 * CS 241 - Spring 2022
 */
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "queue.h"

typedef struct starter {
    queue* q;
    void* data;
} starter;

void* thread_queue_push(void* s) {
    starter* st = (starter*) s;
    queue_push(st->q, st->data);
    return NULL;
}

void* thread_queue_pull(void* s) {
    starter* st = (starter*) s;
    return queue_pull(st->q);
}

int main(int argc, char **argv) {
    ssize_t max_size = 5;
    pthread_t tids[2*max_size];
    starter s[2*max_size];
    int data[2*max_size];
    int res[2*max_size];
    queue *q = queue_create(max_size);
    for(int i = 0; i < 2*max_size; i++) {
        data[i] = i+1;
        s[i].data = (void*)(data + i);
        s[i].q = q;
        pthread_create(&tids[i], NULL, thread_queue_push, (void*)(&s[i]));
    }


    for(int i = 0; i < max_size; i++) {
        pthread_create(&tids[i], NULL, thread_queue_pull, (void*)(&s[i]));
    }

    void* temp_res;
    for(int i = 0; i < max_size; i++) {
        pthread_join(tids[i], &temp_res);
        res[i] = *((int*)temp_res);
    }

    for(int i = 0; i < max_size; i++) {
        printf("%d\n",res[i]);
    }

    queue_destroy(q);

    return 0;
}
