#ifndef CAI_UTILS_H
#define CAI_UTILS_H



#include "common.h"

#define UTILS_LERP(f64_start, f64_end, f64_percentage) \
    (double)((f64_start) + (double)((f64_end) - (f64_start))*(f64_percentage))

flt_t utils_randflt(flt_t lower_bound, flt_t upper_bound);


typedef struct fltarr_t
{
    flt_t *at;
    usize_t count;
    usize_t capacity;
} fltarr_t;

fltarr_t fltarr_Init(void);
void fltarr_Deinit(fltarr_t *arr);
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



#endif /* CAI_UTILS_H */

