#include "array.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define D_ARRAY_START_SIZE 4

int d_array_create(d_array_t * array, size_t elem_size){
    if(array==NULL) return D_ARRAY_ERROR_INVALID_PARAM;
    if(elem_size==0) return D_ARRAY_ERROR_INVALID_PARAM;

    array->length = 0;
    array->elem_size = elem_size;
    array->size = D_ARRAY_START_SIZE*elem_size;
    array->_begin = calloc(elem_size, D_ARRAY_START_SIZE);
    array->_end = array->_begin;

    if(array->_begin==NULL) return D_ARRAY_ERROR_MEMORY;

    return D_ARRAY_ERROR_OK;
}

int d_array_destroy(d_array_t * array){
    if(array==NULL) return D_ARRAY_ERROR_INVALID_PARAM;
    if(array->size==0) return D_ARRAY_ERROR_UNITIALIZED;
    if(array->_begin==NULL) return D_ARRAY_ERROR_UNITIALIZED;

    free(array->_begin);
    array->size = 0;
    array->_begin = NULL;
    array->_end = NULL;

    return D_ARRAY_ERROR_OK;
}

int d_array_resize(d_array_t * array, size_t newsize){
    if(array==NULL) return D_ARRAY_ERROR_INVALID_PARAM;
    if(array->size==0) return D_ARRAY_ERROR_UNITIALIZED;
    if(array->_begin==NULL) return D_ARRAY_ERROR_UNITIALIZED;

    // Do not resize if smaller then start size
    if(newsize<D_ARRAY_START_SIZE) return D_ARRAY_ERROR_OK;

    void * newmem = calloc(array->elem_size, newsize);
    if(newmem==NULL) return D_ARRAY_ERROR_MEMORY;
    memcpy(newmem, array->_begin, array->size>newsize*array->elem_size ? array->elem_size*newsize : array->size);
    free(array->_begin);
    array->_begin = newmem;
    array->_end = (void*)((uintptr_t)array->_begin+(array->length-1)*array->elem_size);
    array->size = newsize*array->elem_size;

    return D_ARRAY_ERROR_OK;
}

void d_array_insert(d_array_t * array, void * data){
    if(array==NULL) return;
    if(array->_begin==NULL) return;

    // Check if need to resize
    if(array->length*array->elem_size>=array->size){
        int err = d_array_resize(array, array->length*2);
        if(err) return;
    }

    void * newpos = (void*)((uintptr_t)array->_begin + array->length*array->elem_size);
    memcpy(newpos, data, array->elem_size); 
    array->length += 1;
    array->_end = newpos;
}

void * d_array_pop(d_array_t * array){
    if(array==NULL) return NULL;
    if(array->_begin==NULL) return NULL;
    if(array->length==0) return NULL;

    array->length -= 1;
    array->_end = (void*)((uintptr_t)array->_begin + (array->length-1)*array->elem_size);

    // Check if need to resize
    // length+1 is used to ensure that the data of the popped object
    // still exists
    if((array->length+1)*array->elem_size*2 <= array->size){
        int err = d_array_resize(array, array->length+1);
        if(err) return array->_end;
    }

    return (void*)((uintptr_t)array->_end+array->elem_size);
}


void d_array_erase(d_array_t * array, size_t index){
    if(array==NULL) return;
    if(array->_begin==NULL) return;
    if(array->length==0) return;
    if(index<0 && index>array->length-1) return;

    void * dst = (void*)((uintptr_t)array->_begin+array->elem_size*index);
    void * src = (void*)((uintptr_t)array->_begin+array->elem_size*(index+1));
    for(size_t i=index; i<array->length; i++){
        memcpy(dst, src, array->elem_size);
        dst = (void*)((uintptr_t)dst + array->elem_size);
        src = (void*)((uintptr_t)src + array->elem_size);
    }
    array->length -= 1;
}
