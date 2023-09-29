
#include <stdlib.h> /* rand, srand */
#include <time.h> /* time */
#include <string.h> /* memset */

#include "include/level.h"
#include "include/mem.h"



static void randomize(Level_t *level);






Level_t Level_Init(usize_t input_count, usize_t output_count)
{
    CAI_ASSERT(input_count < BITARR_COUNT, 
        "a level's input of a level must be < %d\n", BITARR_COUNT
    );
    CAI_ASSERT(output_count < BITARR_COUNT, 
        "a number of output of a level must be < %d\n", BITARR_COUNT
    );


    Level_t level = {
        .output_count = output_count,
        .inputs = fltarr_Init(),
        .outputs = bitarr_Init(),
        .biases = fltarr_Init(),
        .weights = MEM_ALLOC_ARRAY(
            input_count, sizeof(level.weights[0])
        ),
    };

    fltarr_Reserve(&level.inputs, input_count);
    level.inputs.count = input_count;
    fltarr_Reserve(&level.biases, input_count);
    level.biases.count = input_count;
    for (usize_t i = 0; i < input_count; i++)
    {
        level.weights[i] = fltarr_Init();
        fltarr_Reserve(&level.weights[i], output_count);
        level.weights[i].count = output_count;
    }

    randomize(&level);
    return level;
}


void Level_Deinit(Level_t* level)
{
    fltarr_Deinit(&level->biases);
    for (usize_t i = 0; i < level->inputs.count; i++)
    {
        fltarr_Deinit(&level->weights[i]);
    }
    MEM_FREE_ARRAY(level->weights);
    fltarr_Deinit(&level->inputs);

    memset(level, 0, sizeof *level);
}


bitarr_t Level_FeedInput(Level_t* level)
{
    for (usize_t i = 0; i < level->output_count; i++)
    {
        double sum = 0;
        for (usize_t k = 0; k < level->inputs.count; k++)
        {
            sum += level->inputs.at[k] * level->weights[k].at[i];
        }

        usize_t output = sum > level->biases.at[i];
        bitarr_Set(&level->outputs, i, output);
    }
    return level->outputs;
}





bitarr_t Level_FeedForward(Level_t *level, bitarr_t given_input)
{
    for (usize_t i = 0; i < level->inputs.count; i++)
    {
        level->inputs.at[i] = bitarr_Get(given_input, i);
    }
    return Level_FeedInput(level);
}














static void randomize(Level_t *level)
{
    for (usize_t i = 0; i < level->inputs.count; i++)
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


