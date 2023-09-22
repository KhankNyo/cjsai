
#include <stdlib.h>

#include "include/common.h"
#include "include/mem.h"
#include "include/utils.h"





flt_t utils_randflt(flt_t lower_bound, flt_t upper_bound)
{
    return LERP(lower_bound, upper_bound, rand() / (double)RAND_MAX);
}




fltarr_t fltarr_Init(void)
{
    return (fltarr_t){ 0 };
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




