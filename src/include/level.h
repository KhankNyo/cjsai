#ifndef CJSAI_LEVEL_H
#define CJSAI_LEVEL_H



#include "utils.h"
#include "common.h"


typedef struct Level_t
{
    bitarr_t inputs, outputs;
    int input_count, output_count;
    fltarr_t biases;

    fltarr_t *weights; /* each input connects to output_count amount of output */
} Level_t;


Level_t Level_Init(unsigned input_count, unsigned output_count);
void Level_Deinit(Level_t *level);
/* feeds the given input into the level, 
 * returns:
 *  the level's output after the feed forward */
bitarr_t Level_FeedForward(Level_t *level, bitarr_t given_input);




#endif /* CJSAI_LEVEL_H */

