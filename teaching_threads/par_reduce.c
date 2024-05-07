/**
 * teaching_threads
 * CS 241 - Spring 2022
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "reduce.h"
#include "reducers.h"

/* You might need a struct for each task ... */
typedef struct routine_arg {
    int *list;
    size_t list_len;
    reducer reduce_func; 
    int base_case;
} routine_arg;

/* You should create a start routine for your threads. */

void* start_routine (void* arg){
    int* res = malloc(sizeof(int));
    routine_arg* temp = (routine_arg*) arg;
    *res = reduce(temp->list, temp->list_len, temp->reduce_func, temp->base_case);
    pthread_exit((void*) res);
}


void destroy_thread_lists(int** lists, int len) {
    for(int i = 0; i < len; i++) {
        free(lists[i]);
    }
    free(lists);
}

void destroy_args(routine_arg** args, int len) {
    for(int i = 0; i < len; i++) {
        //free(args[i]->list);
        free(args[i]);
    }
    free(args);
}


int par_reduce(int *list, size_t list_len, reducer reduce_func, int base_case,
               size_t num_threads) {
    /* Your implementation goes here */
    size_t thread_len = list_len/num_threads;
    size_t rem = list_len-thread_len*num_threads;

    int** thread_lists = malloc(sizeof(int*) * num_threads);

    pthread_t tids[num_threads];

    routine_arg** args = malloc(sizeof(routine_arg*) * num_threads);

    int* cur = list;
    size_t cur_len;
    for(int i = 0; i < (int) num_threads; i++) {
        cur_len = thread_len;
        if(rem > 0) {
            cur_len ++;
            rem--;
        }


        thread_lists[i] = malloc(cur_len*sizeof(int));
        memcpy(thread_lists[i], cur, cur_len*sizeof(int));
        
        /*printf("%zu\n",cur_len);
        for(int x = 0; x < (int)cur_len; x++) {
            printf("%d ",thread_lists[i][x]);
        }
        printf("\n\n");*/

        cur += (int) cur_len;
        
        args[i]=malloc(sizeof(routine_arg));

        args[i]->list = thread_lists[i];
        args[i]->list_len = cur_len;
        args[i]->reduce_func = reduce_func;
        args[i]->base_case = base_case;

        pthread_create(tids+i, NULL, start_routine, args[i]);

    }


    int* res = malloc(sizeof(int) * num_threads);

    void* temp_ret=NULL;
    for(int j = 0; j < (int) num_threads; j++) {
        pthread_join(tids[j], &temp_ret);
        res[j] = *((int*)temp_ret);
        free(temp_ret);
        //printf("%d\n",res[j]);
    }
    

    int ret = reduce(res, num_threads, reduce_func, base_case);

    free(res);
    destroy_thread_lists(thread_lists, num_threads);
    destroy_args(args,num_threads);
    return ret;
}
