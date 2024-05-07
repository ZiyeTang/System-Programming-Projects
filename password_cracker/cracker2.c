/**
 * password_cracker
 * CS 241 - Spring 2022
 */
#include "cracker2.h"
#include "format.h"
#include "utils.h"
#include "includes/queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <crypt.h>


typedef struct starter {
    char* passform;
    char* hashval;
    char* user;
    int thread_id;
} starter;

typedef struct ender {
    int hashCount;
    char* password;
    int result;
} ender;

queue* taskqueue = NULL;
int found;
size_t nthread;

char** split(char* tsk, char* str) {
    char ** res  = NULL;
    char *  p    = strtok (tsk, str);
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



void* paralcrack(void* arg) { 
    starter* s = (starter*) arg;

    long startidx;
    long count;
    char* passform = s->passform;
    char* hashval = s->hashval;
    char* user = s->user;
    
    int numletters = getPrefixLength(passform);
    getSubrange(strlen(passform)-numletters, nthread, s->thread_id, &startidx, &count); 

    char* temppassform = malloc(17);
    strcpy(temppassform,passform);

    setStringPosition(temppassform + numletters, startidx);
    v2_print_thread_start(s->thread_id, user, startidx, temppassform);

    struct crypt_data cdata;
    cdata.initialized = 0;
    char* hashed;
    int hashCount = 0;
    ender* e = malloc(sizeof(ender));

    for(long i = 0; i <count; i++) {
        
        if(found) {
            break;
        }
        hashCount++;
        hashed = crypt_r(temppassform, "xx", &cdata);
        
        if(strcmp(hashed,hashval)==0){
            found = 1;
            v2_print_thread_result(s->thread_id, hashCount, 0);
            e->hashCount = hashCount;
            e->password = temppassform;
            e->result = 0;
            return (void*) e;
        }
        incrementString(temppassform);
    }

    if(found) {
        v2_print_thread_result(s->thread_id, hashCount, 1);
    } else {
        v2_print_thread_result(s->thread_id, hashCount, 2);
    }
    

    free(temppassform);
    e->result = 1;
    e->hashCount = hashCount;
    e->password = NULL;
    return (void*) e;
}


int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads
    nthread = thread_count;
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

    
    task = queue_pull(taskqueue);

    char** infos;
    void* ret;
    char* password = NULL;
    double starttime;
    double startcputime;
    int result;
    int totalhashcount;
    starter** s = malloc((int)thread_count*sizeof(starter*));
    while(task) {
        
        totalhashcount = 0;
        result = 1;
        starttime = getTime();
        startcputime = getCPUTime();
        infos = split(task, " ");

        v2_print_start_user(infos[0]);

        found = 0;
        for(size_t i = 0; i<thread_count; i++) {
            *(s+i)=malloc(sizeof(starter));
            (*(s+i))->user = infos[0];
            (*(s+i))->hashval = infos[1];
            (*(s+i))->passform = infos[2];
            (*(s+i))->thread_id = (int)i+1;
            pthread_create(&tid[i], NULL, paralcrack, (void*)(*(s+i)));
        }
        
        ret = NULL;
        for(size_t i = 0; i<thread_count; i++) {
            pthread_join(tid[i], &ret);
            ender* res = (ender*) ret;
            if(res->password!=NULL) {
                password = res->password;
            }
            if(res->result == 0) {
                result = 0;
            }
            totalhashcount += res->hashCount;
            free(ret);
            free(*(s+i));
        }

        v2_print_summary(infos[0], password, totalhashcount, getTime()-starttime, getCPUTime()-startcputime, result);
        
        if(password!=NULL) {
            free(password);
        }

        free(infos);
        free(task);
        task = queue_pull(taskqueue);
    }

    free(s);
    queue_destroy(taskqueue);

    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}
