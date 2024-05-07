/**
 * password_cracker
 * CS 241 - Spring 2022
 */
#include "cracker1.h"
#include "format.h"
#include "utils.h"
#include "includes/queue.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <crypt.h>

queue* taskqueue = NULL;

char** split(char* tsk, char* str) {
    char** res = NULL;
    char*  p = strtok (tsk, str);
    int n_spaces = 0;

    while (p) {
        res = realloc (res, sizeof (char*) * ++n_spaces);
        if (res == NULL)
            exit (-1);
        res[n_spaces-1] = p;

        p = strtok (NULL, str);
    }

    res = realloc (res, sizeof (char*) * (n_spaces+1));
    res[n_spaces] = 0;
    return res;
}



void* crack(void* arg) {

    int id = *((int*)arg);

    struct crypt_data cdata;
    cdata.initialized = 0;
    char *hashed;
    
    char* task = queue_pull(taskqueue);

    int* nsucs = malloc(sizeof(int));
    *nsucs = 0;
    char* temppassform;
    int hashcount;
    int suc;

    long nitr;
    int numletters;

    char** infos;
    char* user;
    char* hashval;
    char* passform;
    while(task) {
        infos = split(task, " ");
        user = infos[0];
        hashval = infos[1];
        passform = infos[2];
        
        v1_print_thread_start(id, user);

        numletters = getPrefixLength(passform);
        nitr = pow(26, (double)(strlen(passform)-numletters));
        
        hashcount=0;
        suc = 0;

        temppassform=malloc(16);
        strcpy(temppassform,passform);
        setStringPosition(temppassform+numletters, 0);

        for(long i = 0; i <nitr; i++) {
            hashcount++;
            hashed = crypt_r(temppassform, "xx", &cdata);
            
            if(strcmp(hashed,hashval)==0){
                v1_print_thread_result(id, user, temppassform, hashcount, getThreadCPUTime(), 0);
                suc = 1;
                (*nsucs)++;
                break;
            }
            incrementString(temppassform);
        }
        free(temppassform);

        if(!suc) {
            v1_print_thread_result(id, user, NULL, hashcount, getThreadCPUTime(), 1);
        }

        free(task);
        free(infos);

        task = queue_pull(taskqueue);
    }
    queue_push(taskqueue, NULL);
    return nsucs;
}


int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads

    taskqueue = queue_create(-1);
    char* task = NULL;
    size_t len = 0;
    int ntasks = 0;
    char* temp;
    while(getline(&task, &len, stdin)!=EOF) {
        temp = malloc(strlen(task));
        task[strlen(task)-1]='\0';
        strcpy(temp,task);
        queue_push(taskqueue, temp);
        ntasks++;
    }
    queue_push(taskqueue, NULL);
    free(task);

    pthread_t tid[thread_count];
    int fakeid[thread_count];
    for(size_t i = 0; i<thread_count; i++) {
        fakeid[i] = (int)i+1;
        pthread_create(&tid[i], NULL, crack,&fakeid[i]);
    }

    void* res = NULL;
    int numRecovered = 0;
    for(size_t i = 0; i<thread_count; i++) {
        pthread_join(tid[i], &res);
        numRecovered += *((int*)res);
        free(res);
    }

    v1_print_summary(numRecovered, ntasks - numRecovered);

    queue_destroy(taskqueue);

    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}
