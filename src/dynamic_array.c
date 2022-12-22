#include "dynamic_array.h"

#include <stdlib.h>
#include <string.h>

d_array_error_t d_array_init(d_array_t * array, size_t elem_size){
    if(array==NULL || elem_size<=0)
        return D_ARRAY_ERROR_PARAM;

    void * array_data = calloc(D_ARRAY_START_SIZE, elem_size);
    if(array_data==NULL)
        return D_ARRAY_ERROR_ALLOC;

    array->size = D_ARRAY_START_SIZE;
    array->elem_size = elem_size;
    array->filled_size = 0;
    array->begin = array_data;
    array->end = array_data + (elem_size*0);

    return D_ARRAY_ERROR_OKAY;
}

d_array_error_t d_array_deinit(d_array_t * array){
    if(array==NULL)
        return D_ARRAY_ERROR_PARAM;
    if(array->begin==NULL)
        return D_ARRAY_ERROR_NOINIT;

    free(array->begin);
    array->size = 0;
    array->elem_size = 0;
    array->filled_size = 0;
    array->begin = NULL;
    array->end = NULL;

    return D_ARRAY_ERROR_OKAY;
}

d_array_error_t d_array_initialized(d_array_t * array){
    if(array==NULL)
        return D_ARRAY_ERROR_PARAM;
    if(array->begin==NULL)
        return D_ARRAY_ERROR_NOINIT;
    return D_ARRAY_ERROR_OKAY;
}

d_array_error_t d_array_insert(d_array_t * array, void * data){
    if(array==NULL || data==NULL)
        return D_ARRAY_ERROR_PARAM;
    if(array->begin==NULL)
        return D_ARRAY_ERROR_NOINIT;

    if(array->filled_size+1 > array->size){
        size_t filled_size_new = array->filled_size;
        d_array_resize(array, array->size*2);
        array->filled_size = filled_size_new;
    }

    void * insert_at = array->begin + (array->filled_size*array->elem_size);
    array->filled_size++;
    array->end = array->begin + (array->elem_size*array->filled_size);

    memcpy(insert_at, data, array->elem_size);

    return D_ARRAY_ERROR_OKAY;  
}

d_array_error_t d_array_erase(d_array_t * array, size_t index){
    if(array==NULL)
        return D_ARRAY_ERROR_PARAM;
    if(array->begin==NULL)
        return D_ARRAY_ERROR_NOINIT;
    if(index>=array->filled_size)
        return D_ARRAY_ERROR_OUTOFBOUNDS;

    size_t to_copy = array->filled_size - index - 1;
    if(to_copy<0)
        return D_ARRAY_ERROR_OUTOFBOUNDS;

    void * dst = array->begin + (array->elem_size*index);
    void * src = array->begin + (array->elem_size*(index+1));
    for(size_t i=0; i<to_copy; i++){
        memcpy(dst, src, array->elem_size);
        dst += array->elem_size;
        src += array->elem_size;
    }
    array->filled_size--;

    return D_ARRAY_ERROR_OKAY;
}

d_array_error_t d_array_resize(d_array_t * array, size_t size){
    if(array==NULL || size==0)
        return D_ARRAY_ERROR_PARAM;
    if(array->begin==NULL)
        return D_ARRAY_ERROR_NOINIT;

    if(size!=array->size){
        size_t to_copy = array->filled_size;
        if(to_copy>size)
            to_copy = size;

        void * new_begin = calloc(size, array->elem_size);
        if(new_begin==NULL)
            return D_ARRAY_ERROR_ALLOC;

        memcpy(new_begin, array->begin, array->elem_size*to_copy);

        free(array->begin);
        
        array->begin = new_begin;
        array->end = new_begin + (array->elem_size*array->filled_size);
    }

    array->size = size;
    array->filled_size = size;

    return D_ARRAY_ERROR_OKAY;  
}

void * d_array_at(d_array_t * array, size_t index){
    if(array==NULL)
        return NULL;
    if(array->begin==NULL)
        return NULL;

    if(index>=array->filled_size)
        return NULL;

    return array->begin + (index*array->elem_size);
}
