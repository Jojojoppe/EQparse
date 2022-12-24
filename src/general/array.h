#ifndef __H_ARRAY
#define __H_ARRAY

#include <stddef.h>

typedef struct{
    size_t length;          // Number of elements in array
    size_t elem_size;       // Size of an element
    size_t size;            // Total size of array in memory
    void * _begin;          // Pointer to first element in array
    void * _end;            // Pointer to last element in array
} d_array_t;

enum{
    D_ARRAY_ERROR_OK = 0,
    D_ARRAY_ERROR_INVALID_PARAM,
    D_ARRAY_ERROR_MEMORY,
    D_ARRAY_ERROR_UNITIALIZED,
};

#define D_ARRAY_CREATE(T, array) d_array_create(array, sizeof(T))
#define D_ARRAY_TOP(T, array) ((array)->length>0 ? (T*)(array)->_end : NULL)
#define D_ARRAY_AT(T, array, i) (i<(array)->length ? (T*)(array)->_begin+i : NULL)
#define D_ARRAY_BEGIN(T, array) ((T*)(array)->_begin)
#define D_ARRAY_END(T, array) ((T*)(array)->_end)

#define D_ARRAY_FOREACH_BT(T, var, array) for(T * var=(T*)(array)->_begin; var<=(T*)(array)->_end; var++)
#define D_ARRAY_FOREACH_TB(T, var, array) for(T * var=(T*)(array)->_end; var>=(T*)(array)->_begin; var--)

#define D_ARRAY_POP_DATA(T, array) ((T*)d_array_pop(array))

int d_array_create(d_array_t * array, size_t elem_size);
int d_array_destroy(d_array_t * array);
int d_array_resize(d_array_t * array, size_t newsize);

void d_array_insert(d_array_t * array, void * data);
void * d_array_pop(d_array_t * array);
void d_array_erase(d_array_t * array, size_t index);

#endif
