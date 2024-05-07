/**
 * extreme_edge_cases
 * CS 241 - Spring 2022
 */
#include "camelCaser.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

char **camel_caser(const char *input_str) {
    // TODO: Implement me!
    if(input_str == NULL) {
        return NULL;
    }
    const char* temp = input_str;
    int numPunct = 0;
    while(*temp) {
        if (ispunct(*temp)) {
            numPunct++;
        }
        temp++;
    }

    char** res = malloc((numPunct+1)*sizeof(char*));
    char** cur = res;
    int newSen = 1;
    int spaceBefore = 0;
    int idx = 0;
    while(*input_str) {
        if(isalpha(*input_str)) {
            if (newSen) {
                *cur = malloc(strlen(input_str)+1);
                *((*cur)+idx) = tolower(*input_str);
                newSen = 0;
                spaceBefore = 0;
            } else if (spaceBefore) {
                *((*cur)+idx) = toupper(*input_str);
                spaceBefore = 0;
            } else {
                *((*cur)+idx) = tolower(*input_str);
            }
            idx++;
        } else if (isspace(*input_str)) {
            spaceBefore = 1;
        } else if (ispunct(*input_str)) {
            if(newSen) {
                *cur = malloc(strlen(input_str)+1);
            }
            *((*cur)+idx) = '\0';
            newSen = 1;
            idx = 0;
            cur++;
        } else {
            if (newSen) {
                *cur = malloc(strlen(input_str));
                newSen = 0;
                spaceBefore = 0;
            }
            *((*cur)+idx) = *input_str;
            idx++;
        }
        input_str++;
    }
    *cur = NULL;
    return res;
    //return NULL;
}

void destroy(char **result) {
    // TODO: Implement me!
    if(result==NULL) {
        return;
    }
    char** res = result;
    while(*res!=NULL) {
        free(*res);
        *res = NULL;
        res++;
    }
    free(result);
    result = NULL;
}
