#ifndef CAI_LEVEL_H
#define CAI_LEVEL_H



#include "utils.h"
#include "common.h"



typedef struct Level_t
{
    fltarr_t inputs;
    bitarr_t outputs;
    usize_t output_count;
    fltarr_t biases;
    fltarr_t* weights;
} Level_t;


Level_t Level_Init(usize_t input_count, usize_t output_count);
void Level_Deinit(Level_t* level);

/* acts on the inputs field */
bitarr_t Level_FeedInput(Level_t *level);
bitarr_t Level_FeedForward(Level_t *level, bitarr_t given_input);




#endif /* CAI_LEVEL_H */

