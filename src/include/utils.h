#ifndef CAI_UTILS_H
#define CAI_UTILS_H


#include <math.h>
#include <raylib.h>

#include "common.h"
#include "float.h"




enum 
{
    ENDIAN_BIG = 0x03020100,
    ENDIAN_LITTLE = 0x00010203,
    ENDIAN_PDP = 0x01000203,
    ENDIAN_HONEYWELL = 0x020300001,
};

typedef union u32bytes_t
{
    uint8_t bytes[4];
    uint32_t val;
} u32bytes_t;
extern const u32bytes_t g_EndianOrder;
#define HOST_ENDIAN() (g_EndianOrder.val)



#define LERP(fstart, fend, fpercentage) f32lerp(fstart, fend, fpercentage)
#define ZEROALL(type) (type){0}
#define DEG_TO_RAD(deg) ((deg) * (PI / 180.0f))
#define STATIC_ARRSIZE(array) (sizeof(array) / sizeof(array[0]))
#define BYTE_PERCENTAGE(percentage) ((uint8_t)((double)((percentage))*(double)UINT8_MAX))
#define VEC0 (Vector2){0}



flt_t utils_randflt(flt_t lower_bound, flt_t upper_bound);
flt_t f32lerp(flt_t start, flt_t end, flt_t percentage);


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
    unsigned i = index % BITS_IN_WORD;
    unsigned slot = index / BITS_IN_WORD;

    value = value != 0; /* ensure that value is 1 or 0 */
    value <<= i;
    uintptr_t prev = bitarray->bits[slot]; /* get previous bit for return */
    uintptr_t unset = prev & ~(1 << i);

    bitarray->bits[slot] = (unset | value); /* set the bit */
    return (prev >> i) & 1;
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

