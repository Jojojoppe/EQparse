#ifndef __H_STACK
#define __H_STACK

#include <stddef.h>

typedef struct{
    size_t length;          // Number of elements in stack
    size_t elem_size;       // Size of an element
    size_t size;            // Total size of stack in memory
    void * _begin;          // Pointer to first element in stack
    void * _end;            // Pointer to last element in stack
} d_stack_t;

enum{
    D_STACK_ERROR_OK = 0,
    D_STACK_ERROR_INVALID_PARAM,
    D_STACK_ERROR_MEMORY,
    D_STACK_ERROR_UNITIALIZED,
};

#define D_STACK_CREATE(T, stack) d_stack_create(stack, sizeof(T))
#define D_STACK_TOP(T, stack) ((stack)->length>0 ? (T*)(stack)->_end : NULL)
#define D_STACK_AT(T, stack, i) (i<(stack)->length ? (T*)(stack)->_begin+i : NULL)
#define D_STACK_BEGIN(T, stack) ((T*)(stack)->_begin))
#define D_STACK_END(T, stack) ((T*)(stack)->_end))

#define D_STACK_FOREACH_BT(T, var, stack) for(T * var=(T*)(stack)->_begin; var<=(T*)(stack)->_end; var++)
#define D_STACK_FOREACH_TB(T, var, stack) for(T * var=(T*)(stack)->_end; var>=(T*)(stack)->_begin; var--)

#define D_STACK_POP_DATA(T, stack) ((T*)d_stack_pop(stack))

int d_stack_create(d_stack_t * stack, size_t elem_size);
int d_stack_destroy(d_stack_t * stack);
int d_stack_resize(d_stack_t * stack, size_t newsize);

void d_stack_push(d_stack_t * stack, void * data);
void * d_stack_pop(d_stack_t * stack);

#endif
