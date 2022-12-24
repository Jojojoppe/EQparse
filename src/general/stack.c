#include "stack.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define D_STACK_START_SIZE 4

int d_stack_create(d_stack_t * stack, size_t elem_size){
    if(stack==NULL) return D_STACK_ERROR_INVALID_PARAM;
    if(elem_size==0) return D_STACK_ERROR_INVALID_PARAM;

    stack->length = 0;
    stack->elem_size = elem_size;
    stack->size = D_STACK_START_SIZE*elem_size;
    stack->_begin = calloc(elem_size, D_STACK_START_SIZE);
    stack->_end = stack->_begin;

    if(stack->_begin==NULL) return D_STACK_ERROR_MEMORY;

    return D_STACK_ERROR_OK;
}

int d_stack_destroy(d_stack_t * stack){
    if(stack==NULL) return D_STACK_ERROR_INVALID_PARAM;
    if(stack->size==0) return D_STACK_ERROR_UNITIALIZED;
    if(stack->_begin==NULL) return D_STACK_ERROR_UNITIALIZED;

    free(stack->_begin);
    stack->size = 0;
    stack->_begin = NULL;
    stack->_end = NULL;

    return D_STACK_ERROR_OK;
}

int d_stack_resize(d_stack_t * stack, size_t newsize){
    if(stack==NULL) return D_STACK_ERROR_INVALID_PARAM;
    if(stack->size==0) return D_STACK_ERROR_UNITIALIZED;
    if(stack->_begin==NULL) return D_STACK_ERROR_UNITIALIZED;

    // Do not resize if smaller then start size
    if(newsize<D_STACK_START_SIZE) return D_STACK_ERROR_OK;

    void * newmem = calloc(stack->elem_size, newsize);
    if(newmem==NULL) return D_STACK_ERROR_MEMORY;
    memcpy(newmem, stack->_begin, stack->size>newsize*stack->elem_size ? stack->elem_size*newsize : stack->size);
    free(stack->_begin);
    stack->_begin = newmem;
    stack->_end = (void*)((uintptr_t)stack->_begin+(stack->length-1)*stack->elem_size);
    stack->size = newsize*stack->elem_size;

    return D_STACK_ERROR_OK;
}

void d_stack_push(d_stack_t * stack, void * data){
    if(stack==NULL) return;
    if(stack->_begin==NULL) return;

    // Check if need to resize
    if(stack->length*stack->elem_size>=stack->size){
        int err = d_stack_resize(stack, stack->length*2);
        if(err) return;
    }

    void * newpos = (void*)((uintptr_t)stack->_begin + stack->length*stack->elem_size);
    memcpy(newpos, data, stack->elem_size); 
    stack->length += 1;
    stack->_end = newpos;
}

void * d_stack_pop(d_stack_t * stack){
    if(stack==NULL) return NULL;
    if(stack->_begin==NULL) return NULL;
    if(stack->length==0) return NULL;

    stack->length -= 1;
    stack->_end = (void*)((uintptr_t)stack->_begin + (stack->length-1)*stack->elem_size);

    // Check if need to resize
    // length+1 is used to ensure that the data of the popped object
    // still exists
    if((stack->length+1)*stack->elem_size*2 <= stack->size){
        int err = d_stack_resize(stack, stack->length+1);
        if(err) return stack->_end;
    }

    return (void*)((uintptr_t)stack->_end+stack->elem_size);
}
