
#include <stdlib.h> /* rand, srand */
#include <time.h> /* time */
#include <string.h> /* memset */

#include "include/level.h"
#include "include/mem.h"



static void randomize(Level_t *level);


Level_t Level_Init(unsigned input_count, unsigned output_count)
{

    CAI_ASSERT(input_count < BITARR_COUNT, 
        "a level's input of a level must be < %d\n", BITARR_COUNT
    );
    CAI_ASSERT(output_count < BITARR_COUNT, 
        "a number of output of a level must be < %d\n", BITARR_COUNT
    );

    Level_t level = {
        .inputs = bitarr_Init(),
        .outputs = bitarr_Init(),

        .input_count = input_count,
        .output_count = output_count,

        .weights = MEM_ALLOC_ARRAY(
            input_count, sizeof(level.weights[0])
        ),
        .biases = fltarr_Init(),
    };
    fltarr_Reserve(&level.biases, input_count);

    for (usize_t i = 0; i < input_count; i++)
    {
        level.weights[i] = fltarr_Init();
        fltarr_Reserve(&level.weights[i], output_count);
    }
    
    randomize(&level);
    return level;
}


void Level_Deinit(Level_t *level)
{
    fltarr_Deinit(&level->biases);
    for (usize_t i = 0; i < level->input_count; i++)
    {
        fltarr_Deinit(&level->weights[i]);
    }
    MEM_FREE_ARRAY(level->weights);

    bitarr_Deinit(&level->inputs);
    bitarr_Deinit(&level->outputs);

    level->input_count = 0;
    level->output_count = 0;
}







bitarr_t Level_FeedForward(Level_t *level, bitarr_t given_input)
{
    for (usize_t i = 0; i < level->input_count; i++)
    {
        unsigned input = bitarr_Get(given_input, i);
        bitarr_Set(&level->inputs, i, input);
    }

    for (usize_t i = 0; i < level->output_count; i++)
    {
        /* accumulate the inputs while
         * accounting for weights of the output 
         */
        double sum = 0;
        for (usize_t j = 0; j < level->input_count; j++)
        {
            if (bitarr_Get(level->inputs, j))
                sum += level->weights[j].at[i];
        }

        unsigned output = sum > level->biases.at[i];
        bitarr_Set(&level->outputs, i, output);
    }
    return level->outputs;
}














static void randomize(Level_t *level)
{
    srand(time(NULL));
    for (usize_t i = 0; i < level->input_count; i++)
    {
        for (usize_t j = 0; j < level->output_count; j++)
        {
            level->weights[i].at[j] = utils_randflt(-1, 1);
        }
    }

    for (usize_t i = 0; i < level->biases.count; i++)
    {
        level->biases.at[i] = utils_randflt(-1, 1);
    }
}


