
#include <stdlib.h>

#include "include/common.h"
#include "include/mem.h"
#include "include/utils.h"





static Reading_t *segment_intersect(
    Reading_t *reading, 
    Vector2 A, Vector2 B, Vector2 C, Vector2 D
);
static Vector2 rotate_point(Vector2 pt, Vector2 origin, double rad);


flt_t utils_randflt(flt_t lower_bound, flt_t upper_bound)
{
    return LERP(lower_bound, upper_bound, rand() / (double)RAND_MAX);
}

flt_t f32lerp(flt_t start, flt_t end, flt_t percentage)
{
    return start + (end - start) * percentage;
}


Reading_t *Line_Intersect(Reading_t *reading, const Line_t a, const Line_t b)
{
    return segment_intersect(reading, a.start, a.end, b.start, b.end);
}



static Reading_t *segment_intersect(Reading_t *reading, Vector2 A, Vector2 B, Vector2 C, Vector2 D)
{
    flt_t t_top = (D.x-C.x)*(A.y-C.y)-(D.y-C.y)*(A.x-C.x);
    flt_t u_top = (C.y-A.y)*(A.x-B.x)-(C.x-A.x)*(A.y-B.y);
    flt_t bottom = (D.y-C.y)*(B.x-A.x)-(D.x-C.x)*(B.y-A.y);

    if (bottom)
    {
        flt_t t = t_top / bottom;
        flt_t u = u_top / bottom;
        if (flt_inrange_inclusive(0, t, 1) && flt_inrange_inclusive(0, u, 1))
        {
            reading->dist = t;
            reading->at.x = LERP(A.x, B.x, t);
            reading->at.y = LERP(A.y, B.y, t);
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



Line_t Line_Rotate(Line_t line, Vector2 origin, double rad)
{
    return (Line_t){
        .start = rotate_point(line.start, origin, rad),
        .end = rotate_point(line.end, origin, rad),
    };
}


bool Line_PolyCollide(Line_t* poly1, usize_t count1, Line_t* poly2, usize_t count2)
{
    for (usize_t i = 0; i < count1; i++)
    {
        for (usize_t k = 0; k < count2; k++)
        {
            Reading_t reading;
            if (Line_Intersect(&reading, poly1[i], poly2[k]))
                return true;
        }
    }
    return false;
}






static Vector2 rotate_point(Vector2 pt, Vector2 org, double rad)
{
    Vector2 diff = {
        .x = pt.x - org.x,
        .y = pt.y - org.y
    };
    return (Vector2) {
        .x = org.x + (cos(rad)*diff.x - sin(rad)*diff.y),
        .y = org.y + (sin(rad)*diff.x + cos(rad)*diff.y),
    };
}





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




