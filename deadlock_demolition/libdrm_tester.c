/**
 * deadlock_demolition
 * CS 241 - Spring 2022
 */
#include "libdrm.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static pthread_t pthreads[10];

typedef struct starter {
    drm_t* drm1;
    drm_t* drm2;
} starter;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void* func1(void* arg) {
    starter* s = (starter*) arg;
    pthread_t tid = pthread_self();
    int a = drm_wait(s->drm1, &tid);
    printf("t1l1: %d\n",a);
    int b = drm_wait(s->drm2, &tid);
    printf("t1l2: %d\n",b);
    drm_post(s->drm2, &tid);
    drm_post(s->drm1, &tid);
    return NULL;
}

void* func2(void* arg) {
    starter* s = (starter*) arg;
    pthread_t tid = pthread_self();
    
    int a = drm_wait(s->drm2, &tid);
    printf("t2l2: %d\n",a);
    int b = drm_wait(s->drm1, &tid);
    printf("t2l1: %d\n",b);
    drm_post(s->drm1, &tid);
    drm_post(s->drm2, &tid);
    return NULL;
}


int main() {
    
    // TODO your tests here
    void* ret;
    starter s;
    s.drm1 = drm_init();
    s.drm2 = drm_init();
    pthread_create(&pthreads[0], NULL, &func1, (void*)(&s));
    pthread_create(&pthreads[1], NULL, &func2, (void*)(&s));
    pthread_join(pthreads[0], &ret);
    pthread_join(pthreads[1], &ret);
    drm_destroy(s.drm1);
    drm_destroy(s.drm2);
    return 0;
}
