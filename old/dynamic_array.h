#ifndef __H_DYNAMIC_ARRAY
#define __H_DYNAMIC_ARRAY

#include <stddef.h>

#define D_ARRAY_START_SIZE 4

typedef enum D_ARRAY_ERROR{
    D_ARRAY_ERROR_OKAY = 0,
    D_ARRAY_ERROR_PARAM,
    D_ARRAY_ERROR_ALLOC,
    D_ARRAY_ERROR_NOINIT,
    D_ARRAY_ERROR_OUTOFBOUNDS,
} d_array_error_t;

typedef struct{
    size_t size;
    size_t elem_size;
    size_t filled_size;
    void * begin;
    void * end;
} d_array_t;

#define D_ARRAY_INIT(T, array) d_array_init(array, sizeof(T))
d_array_error_t d_array_init(d_array_t * array, size_t elem_size);

d_array_error_t d_array_deinit(d_array_t * array);

d_array_error_t d_array_initialized(d_array_t * array);

d_array_error_t d_array_insert(d_array_t * array, void * data);

d_array_error_t d_array_erase(d_array_t * array, size_t index);

d_array_error_t d_array_resize(d_array_t * array, size_t size);

void * d_array_at(d_array_t * array, size_t index);

#define D_ARRAY_ATV(T, array, i) (*(T*)d_array_at(array, i))
#define D_ARRAY_ATP(T, array, i) ((T*)d_array_at(array, i))
#define D_ARRAY_DP(T, array) ((T*)array.begin)
#define D_ARRAY_PDP(T, array) ((T*)array->begin)
#define D_ARRAY_LEN(array) array.filled_size
#define D_ARRAY_PLEN(array) array->filled_size
#define D_ARRAY_END(T, array) ((T*)d_array_at(&array, D_ARRAY_LEN(array)-1))
#define D_ARRAY_PEND(T, array) ((T*)d_array_at(array, D_ARRAY_PLEN(array)-1))

#define D_ARRAY_LOOP(T, var, array) T * var=(T*)array.begin; var<(T*)array.end; var++
#define D_ARRAY_PLOOP(T, var, array) T * var=(T*)array->begin; var<(T*)array->end; var++

#endif
