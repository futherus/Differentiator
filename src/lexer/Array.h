#ifndef ARRAY_H
#define ARRAY_H

#ifndef __USE_MINGW_ANSI_STDIO
#define __USE_MINGW_ANSI_STDIO 1
#endif
#include <stdio.h>
#include <stdint.h>

const size_t ARRAY_CHUNK_SIZE = 4096;
const size_t ARRAY_PTR_ARR_MIN_CAP = 8;

struct Array
{
    char*  main_buffer    = nullptr;
    size_t main_buffer_sz = 0;

    char** ptr_arr     = nullptr;
    size_t ptr_arr_cap = 0;

    size_t size = 0;
    size_t cap  = 0;
};

enum array_err
{
    ARRAY_NOERR     = 0,
    ARRAY_NULLPTR   = 1,
    ARRAY_BAD_ALLOC = 2,
    ARRAY_REINIT    = 3,
    ARRAY_NOTINIT   = 4,
};

array_err array_init(Array* arr, size_t main_buffer_sz);
array_err array_dstr(Array* arr);

array_err array_add(Array* arr, const char** ptr, const char data[], size_t data_sz = 0);

#endif // ARRAY_H
