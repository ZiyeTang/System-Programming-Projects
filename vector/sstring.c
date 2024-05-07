/**
 * vector
 * CS 241 - Spring 2022
 */
#include "sstring.h"
#include "vector.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <string.h>

struct sstring {
    // Anything you want
    int length;
    int capacity;
    char* str;
};


static int sstr_new_capacity(int target) {
    int new_capacity = 1;
    while (new_capacity < target) {
        new_capacity *= GROWTH_FACTOR;
    }
    return new_capacity;
}


sstring *cstr_to_sstring(const char *input) {
    // your code goes here
    assert(input);
    sstring* myStr = malloc(2 * sizeof(int) + sizeof(char*));
    myStr->length = strlen(input);
    myStr->capacity = 2 * myStr->length;
    myStr->str = malloc(myStr->capacity + 1);
    strcpy(myStr->str, input);
    return myStr;
}

char *sstring_to_cstr(sstring *input) {
    // your code goes here
    assert(input);
    char* result = malloc(input->length+1);
    strcpy(result,input->str);
    return result;
}

int sstring_append(sstring *this, sstring *addition) {
    // your code goes here
    assert(this!=NULL);
    assert(this->str!=NULL);
    assert(addition!=NULL);
    assert(addition->str!=NULL);
    
    
    if(addition->length + this->length > this->capacity) {
        this->capacity = sstr_new_capacity(addition->length + this->length + 1);
        char* old = this->str;
        this->str = malloc(this->capacity);
        strcpy(this->str, old);
        string_destructor(old);
        old = NULL;
    }

    if(this == addition) {
        char *temp = malloc(strlen(this->str)+1);
        strcpy(temp, this->str);
        strcat(this->str, temp);
        string_destructor(temp);

    } else {
        strcat(this->str, addition->str);
    }

    this->length = this->length + addition->length;
    return this->length;
}

vector *sstring_split(sstring *this, char delimiter) {
    // your code goes here
    assert(this!=NULL);
    assert(this->str != NULL);
    
    vector* sstrVec = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
    char* str = this->str;
    char* cur;
    int newstart=1;
    while(*str) {
        if(*str==delimiter) {
            if(newstart) {
                cur = malloc(strlen(str) + 1);
            }
            cur[strlen(cur)+1]='\0';
            vector_push_back(sstrVec,cur);
            string_destructor(cur);
            cur = NULL;
            newstart = 1;
        } else {
            if(newstart) {
                cur = malloc(strlen(str) + 1);
                newstart = 0;
            }
            cur[strlen(cur)]=*str;
        }
        str++;
    }

    if(newstart) {
        cur = malloc(strlen(str) + 1);
    }
    cur[strlen(cur)+1]='\0';
    vector_push_back(sstrVec,cur);
    string_destructor(cur);
    cur = NULL;
    return sstrVec;
}

int sstring_substitute(sstring *this, size_t offset, char *target,
                       char *substitution) {
    // your code goes here
    assert(this!=NULL);
    assert(this->str!=NULL);
    assert(target!=NULL);
    assert(substitution!=NULL);
    assert(offset>=0);

    int ltarget = strlen(target);
    int lsub = strlen(substitution);

    if (this->length == 0 || ltarget == 0) {
        return -1;
    }
    

    char *found = strstr(this->str + offset, target);

    if(found == NULL) {
        return -1;
    }

    int idx = found - this->str;
    
    int l1 = idx;
    int l2 = this->str + this->length - (found + ltarget - 1);

    char *pre_half = malloc(l1 + 1);
    char *after_half = malloc(l2 + 1);

    memcpy(pre_half, this->str, l1);
    memcpy(after_half, found + ltarget, l2);

    if(this->length - ltarget + lsub > this->capacity) {
        this->capacity = sstr_new_capacity(this->length - ltarget + lsub + 1);
        string_destructor(this->str);
        this->str = malloc(this->capacity);
    }

    strcpy(this->str,pre_half);
    strcat(this->str,substitution);
    strcat(this->str, after_half);

    string_destructor(pre_half);
    string_destructor(after_half);
    return 0;
}

char *sstring_slice(sstring *this, int start, int end) {
    // your code goes here
    assert(this);
    assert(start >= 0);
    assert(end <= this->length);
    assert(start <= end);
    if(start==end) {
        char* res = malloc(1);
        res[0] = '\0';
        return res;
    }
    char* startadr = this->str + start;
    char* substr = malloc(end - start + 1);
    memcpy(substr, startadr, end-start);
    return substr;
}

void sstring_destroy(sstring *this) {
    // your code goes here
    assert(this);
    if(this->str!=NULL) {
        string_destructor(this->str);
        this->str = NULL;
    }
    free(this);
    
}
