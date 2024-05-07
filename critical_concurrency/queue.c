/**
 * critical_concurrency
 * CS 241 - Spring 2022
 */
#include "queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * This queue is implemented with a linked list of queue_nodes.
 */
typedef struct queue_node {
    void *data;
    struct queue_node *next;
} queue_node;

struct queue {
    /* queue_node pointers to the head and tail of the queue */
    queue_node *head, *tail;

    /* The number of elements in the queue */
    ssize_t size;

    /**
     * The maximum number of elements the queue can hold.
     * max_size is non-positive if the queue does not have a max size.
     */
    ssize_t max_size;

    /* Mutex and Condition Variable for thread-safety */
    pthread_cond_t cv;
    pthread_mutex_t m;
};

queue *queue_create(ssize_t max_size) {
    /* Your code here */
    queue* ret = malloc(sizeof(queue));
    ret->head = NULL;
    ret->tail = NULL;
    ret->max_size = max_size;
    ret->size = 0;
    pthread_mutex_init(&(ret->m), NULL);
    pthread_cond_init(&(ret->cv), NULL);
    return ret;
}

void queue_destroy(queue *this) {
    /* Your code here */
    queue_node* cur = this->head;
    queue_node* temp = cur; 
    while(cur!=NULL) {
        cur = cur->next;
        free(temp);
        temp = cur;
    }

    pthread_mutex_destroy(&(this->m));
    pthread_cond_destroy(&(this->cv));
    free(this);

}

void queue_push(queue *this, void *data) {
    /* Your code here */
    pthread_mutex_lock(&(this->m));
    if(this->max_size>0) {
        while (this->size == this->max_size) {
            pthread_cond_wait(&(this->cv), &(this->m));
        }
    }
    
    queue_node* node = malloc(sizeof(queue_node));
    node->data = data;
    node->next = NULL;

    if(this->head == NULL) {
        this->head = node;
        this->tail = node;
    } else {
        this->tail->next = node;
        this->tail = node;
    }
    
    
    this->size++;
    if(this->size != 0) {
        pthread_cond_broadcast(&(this->cv));
    }
    pthread_mutex_unlock(&(this->m));
}

void *queue_pull(queue *this) {
    /* Your code here */
    pthread_mutex_lock(&(this->m));
    while (this->size == 0) {
        pthread_cond_wait(&(this->cv), &(this->m));
    }
    
    void* data = this->head->data;
    queue_node* next = this->head->next;
    free(this->head);
    if(this->size==1) {
        this->head = NULL;
        this->tail = NULL;
    } else {
        this->head = next;
    }
    
    this->size--;
    if(this->max_size > 0 && this->size != this->max_size) {
        pthread_cond_broadcast(&(this->cv));
    }
    pthread_mutex_unlock(&(this->m));
    return data;
}
