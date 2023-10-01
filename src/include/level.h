#ifndef CAI_LEVEL_H
#define CAI_LEVEL_H



#include "utils.h"
#include "common.h"



typedef struct Level_t
{
    fltarr_t inputs;
    fltarr_t biases;
    bitarr_t outputs;

    usize_t output_count;

    fltarr_t* weights;
    usize_t weight_count;
} Level_t;


Level_t Level_Init(usize_t input_count, usize_t output_count);
void Level_Deinit(Level_t *level);
Level_t Level_Copy(Level_t *dst, const Level_t src);

/* acts on the inputs field */
bitarr_t Level_FeedInput(Level_t *level);
bitarr_t Level_FeedForward(Level_t *level, bitarr_t given_input);




#endif /* CAI_LEVEL_H */

