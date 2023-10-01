
#include <string.h> /* memset */

#include "include/network.h"
#include "include/mem.h"
#include "include/level.h"




NeuralNet_t NeuralNet_Init(usize_t *neurons, usize_t neuron_count)
{
    CAI_ASSERT(neuron_count > 1, "There must be at least 2 neurons in a neural network\n");

    NeuralNet_t nn = {
        .levels = MEM_ALLOC_ARRAY(neuron_count, sizeof(nn.levels[0])),
        .count = neuron_count - 1,
    };
    for (usize_t i = 0; i < neuron_count - 1; i++)
    {
        nn.levels[i] = Level_Init(neurons[i], neurons[i + 1]);
    }
    return nn;
}


void NeuralNet_Deinit(NeuralNet_t *nn)
{
    for (usize_t i = 0; i < nn->count; i++)
    {
        Level_Deinit(&nn->levels[i]);
    }
    MEM_FREE_ARRAY(nn->levels);
    memset(nn, 0, sizeof(*nn));
}


void NeuralNet_Copy(NeuralNet_t *dst, const NeuralNet_t src)
{
    if (dst->count != src.count)
    {
        dst->levels = MEM_REALLOC_ARRAY(dst->levels, src.count, sizeof(dst->levels[0]));
        for (usize_t i = dst->count; i < src.count; i++)
        {
            dst->levels[i] = Level_Copy(NULL, src.levels[i]);
        }
    }

    for (usize_t i = 0; i < dst->count; i++)
    {
        Level_Copy(&dst->levels[i], src.levels[i]);
    }
    dst->count = src.count;
}







bitarr_t NeuralNet_FeedForward(NeuralNet_t *nn)
{
    bitarr_t output = Level_FeedInput(&nn->levels[0]);
    for (usize_t i = 1; i < nn->count; i++)
    {
        output = Level_FeedForward(&nn->levels[i], output);
    }
    return output;
}


void NeuralNet_Mutate(NeuralNet_t *nn, const NeuralNet_t src, double similarity)
{
    NeuralNet_Copy(nn, src);

    for (usize_t i = 0; i < nn->count; i++)
    {
        for (usize_t j = 0; j < nn->levels[i].biases.count; j++)
        {
            nn->levels[i].biases.at[j] = LERP(
                src.levels[i].biases.at[j],
                utils_randflt(-1, 1),
                similarity
            );
        }


        for (usize_t j = 0; j < nn->levels[i].inputs.count; j++)
        {
            for (usize_t k = 0; k < nn->levels[i].output_count; k++)
            {
                nn->levels[i].weights[j].at[k] = LERP(
                    src.levels[i].weights[j].at[k],
                    utils_randflt(-1, 1),
                    similarity
                );
            }
        }
    }
}


