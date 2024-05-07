/**
 * vector
 * CS 241 - Spring 2022
 */
#include "vector.h"
#include <assert.h>
#include <stdio.h>

/**
 * 'INITIAL_CAPACITY' the initial size of the dynamically.
 */
const size_t INITIAL_CAPACITY = 8;
/**
 * 'GROWTH_FACTOR' is how much the vector will grow by in automatic reallocation
 * (2 means double).
 */
const size_t GROWTH_FACTOR = 2;

struct vector {
    /* The function callback for the user to define the way they want to copy
     * elements */
    copy_constructor_type copy_constructor;

    /* The function callback for the user to define the way they want to destroy
     * elements */
    destructor_type destructor;

    /* The function callback for the user to define the way they a default
     * element to be constructed */
    default_constructor_type default_constructor;

    /* Void pointer to the beginning of an array of void pointers to arbitrary
     * data. */
    void **array;

    /**
     * The number of elements in the vector.
     * This is the number of actual objects held in the vector,
     * which is not necessarily equal to its capacity.
     */
    size_t size;

    /**
     * The size of the storage space currently allocated for the vector,
     * expressed in terms of elements.
     */
    size_t capacity;
};

/**
 * IMPLEMENTATION DETAILS
 *
 * The following is documented only in the .c file of vector,
 * since it is implementation specfic and does not concern the user:
 *
 * This vector is defined by the struct above.
 * The struct is complete as is and does not need any modifications.
 *
 * The only conditions of automatic reallocation is that
 * they should happen logarithmically compared to the growth of the size of the
 * vector inorder to achieve amortized constant time complexity for appending to
 * the vector.
 *
 * For our implementation automatic reallocation happens when -and only when-
 * adding to the vector makes its new  size surpass its current vector capacity
 * OR when the user calls on vector_reserve().
 * When this happens the new capacity will be whatever power of the
 * 'GROWTH_FACTOR' greater than or equal to the target capacity.
 * In the case when the new size exceeds the current capacity the target
 * capacity is the new size.
 * In the case when the user calls vector_reserve(n) the target capacity is 'n'
 * itself.
 * We have provided get_new_capacity() to help make this less ambigious.
 */

static size_t get_new_capacity(size_t target) {
    /**
     * This function works according to 'automatic reallocation'.
     * Start at 1 and keep multiplying by the GROWTH_FACTOR untl
     * you have exceeded or met your target capacity.
     */
    size_t new_capacity = 1;
    while (new_capacity < target) {
        new_capacity *= GROWTH_FACTOR;
    }
    return new_capacity;
}

vector *vector_create(copy_constructor_type copy_constructor,
                      destructor_type destructor,
                      default_constructor_type default_constructor) {
    // your code here
    vector* myVec = malloc(sizeof(size_t)*2 + sizeof(void*) + sizeof(copy_constructor_type)
     + sizeof(destructor_type) + sizeof(default_constructor_type));

    if(copy_constructor == NULL || destructor == NULL || default_constructor == NULL) {
        myVec ->  copy_constructor = shallow_copy_constructor;
        myVec -> destructor = shallow_destructor;
        myVec -> default_constructor = shallow_default_constructor;
    }
    else {
        myVec ->  copy_constructor = copy_constructor;
        myVec -> destructor = destructor;
        myVec -> default_constructor = default_constructor;
    }
    
    myVec -> size = 0;
    myVec -> capacity = INITIAL_CAPACITY;
    myVec -> array = malloc(INITIAL_CAPACITY * sizeof(void*));
    // Casting to void to remove complier error. Remove this line when you are
    // ready.
    //(void)INITIAL_CAPACITY;
    //(void)get_new_capacity;
    return myVec;
}

void vector_destroy(vector *this) {
    assert(this);
    // your code here
    void **arr = this->array;
    if(arr!=NULL) {
        for(size_t i = 0; i < this->size; i++) {
            if(*arr!=NULL) {
                this->destructor(*arr);
                *arr = NULL;
            }
            arr++;
        }
        if(this->array!=NULL) {
            free(this->array);
            this->array=NULL;
        }
        
    }
    free(this);
    this = NULL;
}

void **vector_begin(vector *this) {
    assert(this);
    return this->array + 0;
}

void **vector_end(vector *this) {
    assert(this);
    return this->array + this->size;
}

size_t vector_size(vector *this) {
    assert(this);
    // your code here
    return this->size;
}

void vector_resize(vector *this, size_t n) {
    assert(this);
    // your code here
    if(n==this->size) return;

    if (n < this->size) {
        void** arr = this -> array + n;
        if (arr!=NULL) {
            for(size_t i = n; i < this->size; i++) {
                if(*arr!=NULL) {
                    this->destructor(*arr);
                    *arr = NULL;
                }
                arr++;
            }
        }  
    } else if (n > this->size && n <= this->capacity) {
        void** arr = this -> array + this-> size;
        if (arr != NULL) {
            for(size_t i = this -> size; i < n; i++) {
                *arr = this->default_constructor();
                arr++;
            }
        } 
    } else if (n > this -> capacity) {
        void** old_arr = this->array;
        this->capacity = get_new_capacity(n);
        this -> array = malloc(this->capacity * sizeof(void*));

        void** cur_old = old_arr;
        void** cur_new = this->array;
        size_t i;
        
        for (i = 0; i < this->size; i++) {
            *cur_new = *cur_old;
            cur_new++;
            cur_old++;
        }
        
        
        for (; i<n; i++) {
            *cur_new = this->default_constructor();
            cur_new++;
        }

        if(old_arr!=NULL) {
            free(old_arr);
            old_arr = NULL;
        }
    }

    this->size = n;
}

size_t vector_capacity(vector *this) {
    assert(this);
    // your code here
    return this->capacity;
}

bool vector_empty(vector *this) {
    assert(this);
    // your code here
    return this->size == 0;
}

void vector_reserve(vector *this, size_t n) {
    assert(this);
    // your code here
    if (n == this->capacity) {
        return;
    }
    void** old_arr = this->array;
    this->capacity = get_new_capacity(n);
    this -> array = malloc(this->capacity * sizeof(void*));

    void** cur_old = old_arr;
    void** cur_new = this->array;
    size_t i;
        
    for (i = 0; i < this->size; i++) {
        *cur_new = *cur_old;
        cur_new++;
        cur_old++;
    }

    if(old_arr!=NULL) {
        free(old_arr);
        old_arr = NULL;
    }
    
}

void **vector_at(vector *this, size_t position) {
    assert(this);
    assert(position < this->size);
    assert(position >= 0);
    // your code here
    return this->array + position;
}

void vector_set(vector *this, size_t position, void *element) {
    assert(this);
    assert(position < this->size);
    assert(position >= 0);
    // your code here
    free(*(this->array + position));
    *(this->array + position) = this->copy_constructor(element);
}

void *vector_get(vector *this, size_t position) {
    assert(this);
    assert(position < this->size);
    assert(position >= 0);
    // your code here
    return *(this->array + position);
}

void **vector_front(vector *this) {
    assert(this);
    assert(!vector_empty(this));
    // your code here
    return this->array;
}

void **vector_back(vector *this) {
    assert(this);
    assert(!vector_empty(this));
    // your code here
    return this->array + this->size - 1;
}

void vector_push_back(vector *this, void *element) {
    assert(this);
    assert(element);
    // your code here
    if(this->size == this->capacity) {
        vector_reserve(this, this->size+1);
    }
    *(this->array+this->size) = this->copy_constructor(element);
    this->size++;
}

void vector_pop_back(vector *this) {
    assert(this);
    // your code here
    if(!vector_empty(this)) {
        this->destructor(*(this->array + this->size - 1));
        *(this->array+this->size-1) = NULL;
        this->size--;
    }
    
}

void vector_insert(vector *this, size_t position, void *element) {
    assert(this);
    assert(element);
    assert(position <= this->size);
    assert(position >= 0);
    // your code here
    if(position == this->size) {
        vector_push_back(this,element);
        return;
    }
    if(this->size == this->capacity) {
        vector_reserve(this, this->size+1);
    }

    for (size_t i = this->size; i >position ; i--) {
        *(this->array + i) = *(this->array + i - 1);
    }

    *(this->array + position) = this->copy_constructor(element);
    this->size++;
}


void vector_erase(vector *this, size_t position) {
    assert(this);
    assert(position < vector_size(this));
    assert(position >= 0);
    // your code here
    if(!vector_empty(this)) {
        if(*(this->array + position)!=NULL) {
            this->destructor(*(this->array + position));
            *(this->array + position) = NULL;
        }
        
        for (size_t i = position; i < this->size-1; i++) {
            *(this->array + i) = *(this->array + i + 1);
        }
        *(this->array + this->size - 1) = NULL;
        this->size--;
    }
}

void vector_clear(vector *this) {
    // your code here
    assert(this);
    void **arr = this->array;
    if(arr!=NULL) {
        for(size_t i = 0; i < this->size; i++) {
            if(*arr!=NULL) {
                this->destructor(*arr);
                *arr = NULL;
            }
            arr++;
        }
    }
    this->size = 0;
}

// The following is code generated:
vector *shallow_vector_create() {
    return vector_create(shallow_copy_constructor, shallow_destructor,
                         shallow_default_constructor);
}
vector *string_vector_create() {
    return vector_create(string_copy_constructor, string_destructor,
                         string_default_constructor);
}
vector *char_vector_create() {
    return vector_create(char_copy_constructor, char_destructor,
                         char_default_constructor);
}
vector *double_vector_create() {
    return vector_create(double_copy_constructor, double_destructor,
                         double_default_constructor);
}
vector *float_vector_create() {
    return vector_create(float_copy_constructor, float_destructor,
                         float_default_constructor);
}
vector *int_vector_create() {
    return vector_create(int_copy_constructor, int_destructor,
                         int_default_constructor);
}
vector *long_vector_create() {
    return vector_create(long_copy_constructor, long_destructor,
                         long_default_constructor);
}
vector *short_vector_create() {
    return vector_create(short_copy_constructor, short_destructor,
                         short_default_constructor);
}
vector *unsigned_char_vector_create() {
    return vector_create(unsigned_char_copy_constructor,
                         unsigned_char_destructor,
                         unsigned_char_default_constructor);
}
vector *unsigned_int_vector_create() {
    return vector_create(unsigned_int_copy_constructor, unsigned_int_destructor,
                         unsigned_int_default_constructor);
}
vector *unsigned_long_vector_create() {
    return vector_create(unsigned_long_copy_constructor,
                         unsigned_long_destructor,
                         unsigned_long_default_constructor);
}
vector *unsigned_short_vector_create() {
    return vector_create(unsigned_short_copy_constructor,
                         unsigned_short_destructor,
                         unsigned_short_default_constructor);
}
