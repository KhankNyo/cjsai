#ifndef CAI_UTILS_H
#define CAI_UTILS_H


#include <math.h>
#include <raylib.h>

#include "common.h"
#include "float.h"



#define LERP(fstart, fend, fpercentage) f32lerp(fstart, fend, fpercentage)
#define ZEROALL(type) (type){0}
#define DEG_TO_RAD(deg) ((deg) * (PI / 180.0f))
#define STATIC_ARRSIZE(array) (sizeof(array) / sizeof(array[0]))
#define BYTE_PERCENTAGE(percentage) ((uint8_t)((double)((percentage))*(double)UINT8_MAX))
#define VEC0 (Vector2){0}



flt_t utils_randflt(flt_t lower_bound, flt_t upper_bound);
flt_t f32lerp(flt_t start, flt_t end, flt_t percentage);


typedef struct Line_t
{
    Vector2 start, end;
} Line_t;
typedef struct Reading_t
{
    Vector2 at;
    flt_t dist;
} Reading_t;

/* returns NULL if a & b does not intersect, else return the pointer given */
Reading_t *Line_Intersect(Reading_t *at, const Line_t a, const Line_t b);
Line_t Line_From(flt_t x, flt_t y, flt_t offset_w, flt_t offset_h);
Line_t Line_Scale(Line_t line, double scale);
Line_t Line_Rotate(Line_t line, Vector2 origin, double rad);
bool Line_PolyCollide(Line_t* poly1, usize_t count1, Line_t* poly2, usize_t count2);


typedef struct fltarr_t
{
    flt_t *at;
    usize_t count;
    usize_t capacity;
} fltarr_t;

fltarr_t fltarr_Init(void);
void fltarr_Deinit(fltarr_t *arr);

/* dst can be NULL */
fltarr_t fltarr_Copy(fltarr_t *dst, const fltarr_t src);

/* returns newly pushed elem */
flt_t fltarr_Push(fltarr_t *arr, flt_t number);
void fltarr_Reserve(fltarr_t *arr, usize_t nelem);




#define BITARR_COUNT 128u
typedef struct bitarr_t
{
    uintptr_t bits[BITARR_COUNT / BITS_IN_WORD];
} bitarr_t;

#define bitarr_Init() ((bitarr_t){ 0 })
#define bitarr_Deinit(p_bitarr) ((*(p_bitarr)) = bitarr_Init())
#define bitarr_Copy(p_dst, src) memcpy(p_dst, &(src), BITARR_COUNT / BITS_IN_WORD)
#define bitarr_Reset(p_bitarr) (*(p_bitarr) = bitarr_Init())

static inline unsigned bitarr_Get(bitarr_t bitarray, unsigned index)
{
    CAI_ASSERT(index < BITARR_COUNT, "invalid input");
    return 1 & (bitarray.bits[index / BITS_IN_WORD] >> (index % BITS_IN_WORD));
}

static inline unsigned bitarr_Set(bitarr_t *bitarray, unsigned index, unsigned value)
{
    CAI_ASSERT(index < BITARR_COUNT, "invalid input");

    value = value != 0; /* ensure that value is 1 or 0 */
    uintptr_t prev = bitarray->bits[index / BITS_IN_WORD]; /* get previous bit for return */
    bitarray->bits[index / BITS_IN_WORD] |= (value << (index % BITS_IN_WORD)); /* set the bit */
    return (prev >> (index % BITS_IN_WORD)) & 1;
}








static inline bool flt_inrange(double lower, double number, double upper)
{
    return lower < number && number < upper;
}

static inline bool flt_inrange_inclusive(double lower, double number, double upper)
{
    return lower <= number && number <= upper;
}

static inline bool flt_equ(double a, double b)
{
    return flt_inrange_inclusive(a - FLT_EPSILON, b, a + FLT_EPSILON);
}




#endif /* CAI_UTILS_H */

