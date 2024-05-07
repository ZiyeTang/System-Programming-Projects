/**
 * mini_memcheck
 * CS 241 - Spring 2022
 */
#include "mini_memcheck.h"
#include <stdio.h>
#include <string.h>

meta_data *head = NULL;
size_t total_memory_requested = (size_t)0;
size_t total_memory_freed = (size_t)0;
size_t invalid_addresses = (size_t)0;

void *mini_malloc(size_t request_size, const char *filename,
                  void *instruction) {
    // your code here
    if(request_size == 0) {
        return NULL;
    }

    meta_data* node = malloc(sizeof(meta_data) + request_size);
    if(node == NULL) {
        return NULL;
    }

    node->next = head;
    node->instruction = instruction;
    node->filename = filename;
    node->request_size = request_size;

    head = node;
    total_memory_requested += request_size;
    return node + 1;
}

void *mini_calloc(size_t num_elements, size_t element_size,
                  const char *filename, void *instruction) {
    // your code here
    if (num_elements == 0 || element_size == 0) {
        return NULL;
    }

    meta_data* node = malloc(sizeof(meta_data) + element_size * num_elements);
    
    if(node == NULL) {
        return NULL;
    }

    memset((void*)(node + 1), 0, element_size * num_elements);

    node->next = head;
    node->instruction = instruction;
    node->filename = filename;
    node->request_size = element_size * num_elements;

    head = node;
    total_memory_requested += element_size * num_elements;
    return node + 1;
}

void *mini_realloc(void *payload, size_t request_size, const char *filename,
                   void *instruction) {
    // your code here
    if(payload==NULL && request_size!=0) {
        return mini_malloc(request_size, filename, instruction);
    } else if(payload!=NULL && request_size==0) {
        mini_free(payload);
        return NULL;
    } else if (payload==NULL && request_size==0) {
        return NULL;
    }
    
    if(head == NULL) {
        invalid_addresses++;
        return NULL;
    }
    meta_data* prev = head;
    meta_data* cur = prev->next;

    while(prev!=NULL && cur!=NULL) {
        if(payload == cur+1) {
            if (cur->request_size < request_size) {
                total_memory_requested += request_size - cur->request_size;
                prev->next=cur->next;
                cur->next=NULL;
                meta_data* node = realloc(cur, sizeof(meta_data) + request_size);
                if(node == NULL) {
                    return NULL;
                }

                node->next = head;
                node->instruction = instruction;
                node->filename = filename;
                node->request_size = request_size;

                head = node;
                return node + 1;
            } else if (cur->request_size > request_size) {
                total_memory_freed += cur->request_size - request_size;
                cur->request_size = request_size;
                cur->instruction = instruction;
                cur->filename = filename;
                return payload;
            } else {
                return payload;
            }
        }
        prev = prev->next;
        cur = cur->next;
    }

    if(prev == NULL) {
        invalid_addresses++;
    } else {
        if(payload == head + 1) {
            if (head->request_size < request_size) {
                total_memory_requested += request_size - head->request_size;
                meta_data* temp = head->next;
                head->next=NULL;
                head = realloc(head, sizeof(meta_data) + request_size);
                if(head == NULL) {
                    return NULL;
                }

                head->next = temp;
                head->instruction = instruction;
                head->filename = filename;
                head->request_size = request_size;

                return head + 1;
            } else if (head->request_size > request_size) {
                total_memory_freed += head->request_size - request_size;
                head->request_size = request_size;
                head->instruction = instruction;
                head->filename = filename;
                return payload;
            } else {
                return payload;
            }
        } else {
            invalid_addresses++;
        }
    }
    return NULL;
}

void mini_free(void *payload) {
    // your code here
    if(payload==NULL) {
        return;
    }

    if(head==NULL) {
        invalid_addresses++;
        return;
    }
        

    meta_data* prev = head;
    meta_data* cur = prev->next;

    while(prev!=NULL && cur!=NULL) {
        if(payload==cur+1) {
            total_memory_freed += cur->request_size;
            prev->next=cur->next;
            cur->next=NULL;
            free(cur);
            return;
        }
        prev = prev->next;
        cur = cur->next;
    }

    if(prev==NULL) {
        invalid_addresses++;
    } else {
        if(payload == head + 1) {
            total_memory_freed += head->request_size;
            meta_data* temp = head->next;
            head->next=NULL;
            free(head);
            head = temp;
        } else {
            invalid_addresses++;
        }
    }
}
