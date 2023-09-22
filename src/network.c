
#include <string.h> /* memset */

#include "include/network.h"
#include "include/mem.h"
#include "include/level.h"




NeuralNet_t NeuralNet_Init(usize_t *neurons, usize_t neuron_count)
{
    NeuralNet_t nn = {
        .levels = MEM_ALLOC_ARRAY(neuron_count, sizeof(nn.levels[0])),
        .count = neuron_count,
    };
    
    CAI_ASSERT(neuron_count > 1, "There must be at least 2 neurons in a neural network");
    for (usize_t i = 0; i < neuron_count; i++)
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




bitarr_t NeuralNet_FeedForward(NeuralNet_t *nn, bitarr_t given_input)
{
    bitarr_t output = Level_FeedForward(&nn->levels[0], given_input);
    for (usize_t i = 1; i < nn->count; i++)
    {
        output = Level_FeedForward(&nn->levels[i], output);
    }
    return output;
}


void NeuralNet_Mutate(NeuralNet_t *nn, double similarity)
{
    for (usize_t i = 0; i < nn->count; i++)
    {
        for (usize_t j = 0; j < nn->levels[i].biases.count; j++)
        {
            nn->levels[i].biases.at[j] = LERP(
                nn->levels[i].biases.at[j],
                utils_randflt(-1, 1),
                similarity
            );
        }


        for (usize_t j = 0; j < nn->levels[i].input_count; j++)
        {
            for (usize_t k = 0; k < nn->levels[i].output_count; k++)
            {
                nn->levels[i].weights[j].at[k] = LERP(
                    nn->levels[i].weights[j].at[k],
                    utils_randflt(-1, 1),
                    similarity
                );
            }
        }
    }
}


