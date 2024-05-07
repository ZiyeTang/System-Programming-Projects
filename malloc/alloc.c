/**
 * malloc
 * CS 241 - Spring 2022
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>





typedef struct memory_list {
    void* ptr;
    size_t size;
    int free;
    struct memory_list* next;
    struct memory_list* prev;
    struct memory_list* nextfree;
    struct memory_list* prevfree;
} memory_list;


void remove_node(memory_list* node);
memory_list* block_split(memory_list* node, size_t size);
void block_merge(memory_list* node);
memory_list* add_node_to_tail(size_t size);


static memory_list* head = NULL;
static memory_list* tail = NULL;
static memory_list* emhead = NULL;



void remove_node(memory_list* node) {
    if(node == NULL) {
        return;
    }
    memory_list* prevfree = node->prevfree;
    memory_list* nextfree = node->nextfree;

    if(node == emhead) {
        emhead = nextfree;
    }

    if(prevfree!= NULL) {
        prevfree->nextfree = nextfree;
    }
    if(nextfree != NULL) {
        nextfree->prevfree = prevfree;
    }
}

memory_list* block_split(memory_list* node, size_t size) {
    if(node == NULL || node->size <= size + sizeof(memory_list)) {
        return NULL;
    }
    memory_list* new_node = (memory_list*)((char*)(node+1) + size);
    
    new_node->size = node->size - size - sizeof(memory_list);
    node->size = size;

    new_node->free = 1;
    new_node->ptr = new_node + 1;

    memory_list* temp = node->next;
    node->next = new_node;
    new_node->next = temp;
    
    if(temp!=NULL) {
        temp->prev = new_node;
    }
    new_node->prev = node;

    if(tail == node) {
        tail = new_node;
    }

    new_node->nextfree = emhead;
    new_node->prevfree = NULL;
    if(emhead!=NULL) {
        emhead->prevfree = new_node;
    }
    emhead = new_node;
    return new_node;
}

void block_merge(memory_list* node) {
    if(node==NULL || node->next==NULL) {
        return;
    }

    memory_list* nextnode = node->next;

    if(nextnode == tail) {
        tail = node;
    }

    node->size = node->size + nextnode->size + sizeof(memory_list);
    node->next = nextnode->next;
    if(nextnode->next!=NULL) {
        nextnode->next->prev = node;
    }

    nextnode->size=0;
    nextnode->free=0;
    nextnode->next=NULL;
    nextnode->prev=NULL;
    nextnode->ptr=NULL; 

    remove_node(nextnode);
}


memory_list* add_node_to_tail(size_t size) {
    memory_list* node = sbrk(0);

    if(sbrk(sizeof(memory_list)) == (void*) -1) {
        return NULL;
    }

    node->ptr = sbrk(0);
    
    if(sbrk(size) == (void*) -1) {
        return NULL;
    }

    node->size = size;
    node->free = 0;
    node->next = NULL;
    node->prev = tail;
    
    if(tail!=NULL) {
        tail->next = node;
    } else if(tail==NULL && head == NULL) {
        head = node;
    }

    tail = node;

    return node;
}


/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
    // implement calloc!
    //return malloc(num*size);
    void* ptr = malloc(num * size);
	if(ptr == NULL) {
        return NULL;
    }
  	return memset(ptr, 0, num * size);
}


/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size) {
    // implement malloc!
    memory_list* chosen = NULL;
    memory_list* cur = emhead;
    while (cur!=NULL) {
        if(cur->size >= size) {
            chosen = cur;
            if(chosen->size - size > 64 + sizeof(memory_list)) {
                memory_list* splittednode = block_split(chosen, size);

                memory_list* nextnode = splittednode -> next;
                if(nextnode != NULL && nextnode ->free) {
                    block_merge(splittednode);
                }
            }
            remove_node(chosen);
            chosen->free = 0;
            return chosen->ptr;
        }
        cur = cur->nextfree;
    }
    memory_list* newnode = add_node_to_tail(size);
    return newnode->ptr;
}

/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr) {
    // implement free!
    if(!ptr){
        return;
    }
    memory_list* node = (memory_list*)ptr-1;
    if(node->ptr!=ptr) {
        return;
    }
    node->free = 1;
    

    node->nextfree = emhead;
    node->prevfree = NULL;
    if(emhead!=NULL) {
        emhead->prevfree = node;
    }
    emhead = node;

    memory_list* prevnode = node -> prev;
    memory_list* nextnode = node -> next;

    if(prevnode != NULL && prevnode->free) {
        block_merge(prevnode);
        node = prevnode;
    }

    if(nextnode != NULL && nextnode->free) {
        block_merge(node);
    }

}

/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {
    // implement realloc!
    if(ptr == NULL && size!=0) {
        return malloc(size);
    } else if(ptr != NULL && size == 0) {
        free(ptr);
        return NULL;
    } else if (ptr == NULL && size == 0) {
        return NULL;
    } 

    memory_list* node = (memory_list*)ptr - 1;
    if(node->ptr!=ptr) {
        return NULL;
    }

    
  

    if (node->size < size) {
        if(node==tail) {
            sbrk(size- node->size);
            node->size = size;
            return ptr;
        } else {
            void* new_ptr = malloc(size);
            if(new_ptr == NULL) {
                return NULL;
            }
            memcpy(new_ptr, node->ptr, node->size);
            free(node);
            return new_ptr;
        }
        
    } else if (node->size - size > 64 + sizeof(memory_list)) {
       /* memory_list* splittednode = block_split(node, size);

        memory_list* nextnode = splittednode -> next;
        if(nextnode != NULL && nextnode ->free) {
            block_merge(splittednode);
        }*/
        return ptr;
    } else {
        return ptr;
    }
    
}
