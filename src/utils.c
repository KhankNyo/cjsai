
#include <stdlib.h>

#include "include/common.h"
#include "include/mem.h"
#include "include/utils.h"





flt_t utils_randflt(flt_t lower_bound, flt_t upper_bound)
{
    return LERP(lower_bound, upper_bound, rand() / (double)RAND_MAX);
}


Reading_t *Line_Intersect(Reading_t *reading, const Line_t a, const Line_t b)
{
    flt_t t_top = 
        (b.end.x - b.start.x)*(a.start.y - b.start.y) 
        - (b.end.y - b.start.y)*(a.start.x - b.start.x);
    flt_t u_top = 
        (b.start.y - a.start.y)*(a.start.x - a.end.x) 
        - (b.start.x - a.start.x)*(a.start.y - a.end.y);
    flt_t bottom = 
        (b.end.y - b.start.y)*(a.end.x - a.start.x) 
        - (b.end.x - b.start.x)*(a.end.y - a.start.y);


    if (!flt_equ(bottom, 0))
    {
        double t = t_top / bottom;
        double u = u_top / bottom;
        if (flt_inrange_inclusive(0, t, 1) 
        && flt_inrange_inclusive(0, u, 1))
        {
            reading->dist = t;
            reading->at.x = LERP(a.start.x, a.end.x, t);
            reading->at.y = LERP(a.start.y, a.end.y, t);
            return reading;
        }
    }
    return NULL;
}



Line_t Line_From(flt_t x, flt_t y, flt_t offset_w, flt_t offset_h)
{
    return (Line_t){
        .start = {
            .x = x,
            .y = y,
        },
        .end = {
            .x = x + offset_w,
            .y = y + offset_h,
        },
    };
}


Line_t Line_Scale(Line_t line, double scale)
{
    return (Line_t){
        .start.x = line.start.x * scale, 
        .start.y = line.start.y * scale,
        .end.x = line.end.x * scale,
        .end.y = line.end.y * scale,
    };
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




