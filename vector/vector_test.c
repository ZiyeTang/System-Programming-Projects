/**
 * vector
 * CS 241 - Spring 2022
 */
#include "vector.h"
#include <stdio.h>
#include "callbacks.h"
int main(int argc, char *argv[]) {
    // Write your test cases here
    vector* v1 = vector_create(int_copy_constructor, int_destructor, int_default_constructor);
    int *a = malloc(sizeof(int));
    *a = 5;
    vector_push_back(v1, a);
    int* num;
    
    for(int i = 18; i < 150; i += 3) {
        num = malloc(sizeof(int));
        *num = i; 
        vector_insert(v1, 0, num);
        free(num);
    }

    size_t j;
    
    vector_erase(v1,3);
    vector_erase(v1,4);
    vector_erase(v1,7);

    
    vector_insert(v1,10,a);
    vector_insert(v1,20,a);
    vector_insert(v1,25,a);
    vector_resize(v1,8);
    printf("%zu:\n", vector_size(v1));
    for(j = 0; j != vector_size(v1); j ++) {
        printf("%d\n", *(*((int**)vector_at(v1,j))));
       
    }
    vector_destroy(v1);
    free(a);
    return 0;
}
