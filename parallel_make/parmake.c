/**
 * parallel_make
 * CS 241 - Spring 2022
 */

#include "format.h"
#include "graph.h"
#include "parmake.h"
#include "parser.h"
#include "queue.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

queue* rules = NULL;

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

int isCycle(graph *g, char* key) {
    rule_t* val = graph_get_vertex_value(g, key);
    if(val->data!=NULL) {
        free(val->data);
        val->data=NULL;
        return 1;
    } else {
        vector* v = graph_neighbors(g, key);
        if(vector_empty(v)) {
            vector_destroy(v);
            return 0;
        }

        val->data = malloc(sizeof(int));
        int res = 0;
        for(size_t i = 0; i < vector_size(v); i++) {
            res = res || isCycle(g, vector_get(v, i));
        }
        vector_destroy(v);

        free(val->data);
        val->data=NULL;
        return res;
    }
}

int need_to_run_command(graph* g, char* goal) {
    time_t t1;
    time_t t2;
    struct stat s1;
    struct stat s2;

    int res1 = stat(goal, &s1);
    int res2;
    if(res1) {
        return 1;
    }
	t1 = s1.st_mtime;
    vector* v = graph_neighbors(g, goal);
    int ret = 0;
    for(size_t i = 0; i < vector_size(v); i++) {        
        res2 = stat(vector_get(v, i), &s2);
        if(res2) {
            ret = 1;
            break;
        }
        t2 = s2.st_mtime;
        if(difftime(t1, t2)<0) {
            ret = 1;
            break;
        }
    }
    
    vector_destroy(v);
    return ret;
}


void collect_rules(graph* g, char* goal) {
    if(rules==NULL) {
        rules = queue_create(-1);
    }

    vector* neighbors = graph_neighbors(g, goal);
    for(size_t i = 0; i < vector_size(neighbors); i++) {
        collect_rules(g, vector_get(neighbors, i));
    }

    vector_destroy(neighbors);

    rule_t* rule = graph_get_vertex_value(g, goal);
    if(rule->state==0) {
        queue_push(rules, goal);
        rule->state=-1;
    }
    
    
}

void* satisfy_rule(void* arg) {

    graph* g = (graph*) arg;

    char* target = queue_pull(rules);
    vector* neighbors;
    while(target) {
        pthread_mutex_lock(&m);
        rule_t* rule = graph_get_vertex_value(g, target);
        pthread_mutex_unlock(&m);

        if(rule->state != -1) {
            target = queue_pull(rules);
            continue;
        }

        int next=0;
        neighbors = graph_neighbors(g, target);
        for(size_t i = 0; i < vector_size(neighbors); i++) {
            
            rule_t* r = graph_get_vertex_value(g, vector_get(neighbors, i));
            
            while(r->state == -1 || r->state == 3) {
                //printf("%s %d\n",target, r->state);
                pthread_mutex_lock(&m);
                pthread_cond_wait(&cv, &m);
                pthread_mutex_unlock(&m);
                //printf("%s %d\n",target, r->state);
            }

            if(r->state == 2) {
                rule->state = 2;
                pthread_cond_broadcast(&cv);
                next=1;
                break;
            }
            
        }
        vector_destroy(neighbors);

        if(next){
            target = queue_pull(rules);
            continue;
        }

        
        int need_run = need_to_run_command(g, target);
        pthread_mutex_lock(&m);
        rule->state = 3;
        pthread_mutex_unlock(&m);
       
        int success = 1;
        if(need_run) {
            vector* cmds = rule->commands;
            for(size_t i = 0; i < vector_size(cmds); i++) {
                if(system(vector_get(cmds, i))!=0) {
                    pthread_mutex_lock(&m);
                    rule->state = 2;
                    pthread_mutex_unlock(&m);
                    success = 0;
                    break;
                }
            }
            
        }

        if(success) {
            pthread_mutex_lock(&m);
            rule->state = 1;
            pthread_mutex_unlock(&m);
        }
        if(rule->state!=-1 || rule->state!=3) {
            pthread_cond_broadcast(&cv);
        }
        target = queue_pull(rules);

    }
    queue_push(rules,NULL);
    return NULL;
}

int parmake(char *makefile, size_t num_threads, char **targets) {
    // good luck!
    graph* dg = parser_parse_makefile(makefile, targets);
    vector* goals = graph_neighbors(dg, "");
    
    for (size_t i = 0; i < vector_size(goals); i++) {
        char* goal = vector_get(goals, i);
        if(isCycle(dg, goal)) {
            print_cycle_failure(goal);
            rule_t *r = graph_get_vertex_value(dg, goal);
            r->state = 2;
        } else {
            collect_rules(dg, goal);
        }
    }
    vector_destroy(goals);

    if(rules!=NULL) {
        queue_push(rules, NULL);

        pthread_t tid[num_threads];
        for(size_t i = 0; i < num_threads; i++) {
            pthread_create(&tid[i], NULL, satisfy_rule, dg);
        }

        void* res = NULL;
        for(size_t i = 0; i < num_threads; i++) {
            pthread_join(tid[i], &res);
        }

        queue_destroy(rules);
    }

    graph_destroy(dg);
    return 0;
}
