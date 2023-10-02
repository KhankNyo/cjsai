
#include <stdlib.h>

#include "include/common.h"
#include "include/mem.h"
#include "include/utils.h"












fltarr_t fltarr_Init(void)
{
    return (fltarr_t){ 0 };
}


fltarr_t fltarr_Copy(fltarr_t *dst, const fltarr_t src)
{
    fltarr_t arr;
    if (NULL == dst)
    {
        arr = fltarr_Init();
        fltarr_Reserve(&arr, src.count);
        dst = &arr;
    }
    else if (dst->count < src.count)
    {
        MEM_FREE_ARRAY(dst->at);
        dst->at = MEM_ALLOC_ARRAY(src.count, sizeof(dst->at[0]));
    }

    dst->count = src.count;
    for (usize_t i = 0; i < src.count; i++)
        dst->at[i] = src.at[i];
    return *dst;
}


flt_t fltarr_Push(fltarr_t *arr, flt_t number)
{
    if (arr->count + 1 > arr->capacity)
    {
        fltarr_Reserve(arr, 
            MEM_GROW_CAPACITY(arr->capacity)
        );
    }

    arr->at[arr->count++] = number;
    return number;
}

void fltarr_Reserve(fltarr_t *arr, usize_t nelem)
{
    if (arr->capacity < nelem)
    {
        arr->capacity = nelem;
        arr->at = MEM_REALLOC_ARRAY(arr->at,
            sizeof(arr->at[0]),
            arr->capacity
        );
    }
}

void fltarr_Deinit(fltarr_t *arr)
{
    MEM_FREE_ARRAY(arr->at);
    *arr = fltarr_Init();
}




