/**
 * deadlock_demolition
 * CS 241 - Spring 2022
 */
#include "graph.h"
#include "libdrm.h"
#include "set.h"
#include <pthread.h>
#include <stdio.h>
struct drm_t {
    pthread_mutex_t m;
};

static graph* g;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;


int isCyclic(graph *g, void* startkey) {
    if (*((int *)graph_get_vertex_value(g, startkey))==1) {
        int val = 0;
        graph_set_vertex_value(g, startkey, &val);
        return 1;
    } else {
        
        vector* v = graph_neighbors(g, startkey);
        if(vector_empty(v)) {
            return 0;
        }

        int setval = 1;
        graph_set_vertex_value(g, startkey, &setval);
        int res = 0;
        for(size_t i = 0; i < vector_size(v); i++) {
            res = res || isCyclic(g, vector_get(v, i));
        }

        int val = 0;
        /*for(size_t i = 0; i < vector_size(v); i++) {
            graph_set_vertex_value(g, vector_get(v, i), &val);
        }*/
        
        graph_set_vertex_value(g, startkey, &val);
        return res;

    }
}


drm_t *drm_init() {
    /* Your code here */
    
    if(g==NULL) {
        g = shallow_graph_create();
    }
    drm_t* mydrm = malloc(sizeof(drm_t));
    pthread_mutex_init(&(mydrm->m), NULL);

    pthread_mutex_lock(&mtx);
    int val = 0;
    graph_add_vertex(g, (void*)mydrm);
    graph_set_vertex_value(g, mydrm, &val);
    pthread_mutex_unlock(&mtx);
    return mydrm;
}


int drm_post(drm_t *drm, pthread_t *thread_id) {
    /* Your code here */
    pthread_mutex_lock(&mtx);
    vector* v1 = graph_neighbors(g, (void*)drm);
    
    if(graph_contains_vertex(g, thread_id) && !vector_empty(v1) && vector_get(v1,0) == thread_id) {
        vector* v2 = graph_antineighbors(g, (void*)drm);
        graph_remove_edge(g, drm, thread_id);
        if(!vector_empty(v2)){
            graph_add_edge(g, (void*)drm, vector_get(v2,0));
            graph_remove_edge(g, vector_get(v2,0), drm);
        }
        
        pthread_mutex_unlock(&mtx);
        pthread_mutex_unlock(&(drm->m));
        return 1;
    }
    pthread_mutex_unlock(&mtx);
    return 0;
}


int drm_wait(drm_t *drm, pthread_t *thread_id) {
    /* Your code here */
    pthread_mutex_lock(&mtx);

    
    vector* v1 = graph_neighbors(g, (void*)drm);
    if(!vector_empty(v1) && thread_id == vector_get(v1,0)) {
        return 0;
    }

    if(!graph_contains_vertex(g, thread_id)) {
        graph_add_vertex(g, (void*)thread_id);
        int val = 0;
        graph_set_vertex_value(g, thread_id, &val);
    }

    

    if(vector_empty(v1)) {
        graph_add_edge(g, (void*)drm, (void*)thread_id);
        pthread_mutex_unlock(&mtx);
        pthread_mutex_lock(&(drm->m));
        return 1;
    } else {
        graph_add_edge(g, (void*)thread_id, (void*)drm);
        if(isCyclic(g, thread_id)) {
            graph_remove_edge(g, thread_id, drm);
            pthread_mutex_unlock(&mtx);
            return 0;
        } else {
            pthread_mutex_unlock(&mtx);
            pthread_mutex_lock(&(drm->m));
            return 1;
        }
    }

}

void drm_destroy(drm_t *drm) {
    /* Your code here */
    pthread_mutex_lock(&mtx);
    if(g!=NULL) {
        graph_destroy(g);
        g = NULL;
    }
    pthread_mutex_unlock(&mtx);
    pthread_mutex_destroy(&(drm->m));
    free(drm);
    return;
}
