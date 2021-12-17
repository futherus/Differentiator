#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "Array.h"

// add array dump
#define ASSERT(CONDITION, ERROR)                \
    do                                          \
    {                                           \
        if(!(CONDITION))                        \
        {                                       \
            printf("%s\n", #ERROR);             \
            return (ERROR);                     \
        }                                       \
    } while(0)                                  \

static array_err ptr_arr_resize_(Array* arr)
{
    assert(arr);

    size_t new_cap = arr->ptr_arr_cap;
    if(new_cap == 0)
        new_cap = ARRAY_PTR_ARR_MIN_CAP;

    char** temp = (char**) realloc(arr->ptr_arr, new_cap * sizeof(char));
    ASSERT(temp, ARRAY_BAD_ALLOC);

    memset(temp + arr->ptr_arr_cap, 0, new_cap - arr->ptr_arr_cap);

    arr->ptr_arr     = temp;
    arr->ptr_arr_cap = new_cap;

    return ARRAY_NOERR;
}

static array_err add_chunk_(Array* arr)
{
    assert(arr);

    if(arr->cap / ARRAY_CHUNK_SIZE == arr->ptr_arr_cap)
        if(ptr_arr_resize_(arr))
            return ARRAY_BAD_ALLOC;
    
    char* temp = (char*) calloc(ARRAY_CHUNK_SIZE, sizeof(char));
    ASSERT(temp, ARRAY_BAD_ALLOC);

    arr->ptr_arr[arr->cap / ARRAY_CHUNK_SIZE] = temp;
    arr->cap += ARRAY_CHUNK_SIZE;
    
    return ARRAY_NOERR;
}

array_err array_init(Array* arr, size_t main_buffer_sz)
{
    ASSERT(arr, ARRAY_NULLPTR);

    if(main_buffer_sz == 0)
        return ARRAY_NOERR;
    
    char* temp = (char*) calloc(main_buffer_sz, sizeof(char));
    ASSERT(temp, ARRAY_BAD_ALLOC);

    arr->main_buffer    = temp;
    arr->main_buffer_sz = main_buffer_sz;

    return ARRAY_NOERR;
}

array_err array_dstr(Array* arr)
{
    ASSERT(arr, ARRAY_NULLPTR);

    free(arr->main_buffer);

    for(size_t iter = 0; iter < arr->cap / ARRAY_CHUNK_SIZE; iter++)
        free(arr->ptr_arr[iter]);
    
    return ARRAY_NOERR;
}

array_err array_add(Array* arr, const char** ptr, const char data[], size_t data_sz)
{
    ASSERT(arr && ptr && data, ARRAY_NULLPTR);

    if(data_sz == 0)
        data_sz = strlen(data);

    ASSERT(data_sz < ARRAY_CHUNK_SIZE, ARRAY_CHUNK_OVRFLW);

    if(arr->cap < arr->size + data_sz)
    {
        size_t prev_cap = arr->cap;

        if(add_chunk_(arr))
            return ARRAY_BAD_ALLOC;

        arr->size = prev_cap;
    }

    char* new_string = &arr->ptr_arr[arr->size / ARRAY_CHUNK_SIZE][arr->size % ARRAY_CHUNK_SIZE];
    memcpy(new_string, data, data_sz);
    new_string[data_sz] = '\0';
    *ptr = new_string;

    arr->size += (data_sz + 1);

    return ARRAY_NOERR;
}

