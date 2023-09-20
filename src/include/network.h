#ifndef CJSAI_NETWORK_H
#define CJSAI_NETWORK_H


#include "common.h"
#include "level.h"


typedef struct NeuralNet_t
{
    Level_t *levels;
    usize_t count;
} NeuralNet_t;


NeuralNet_t NeuralNet_Init(usize_t *neurons, usize_t neuron_count);
void NeuralNet_Deinit(NeuralNet_t *nn);

/* a wrapper around level's feed forward */
bitarr_t NeuralNet_FeedForward(NeuralNet_t *nn, bitarr_t given_input);


/* 0 <= similarity <= 1,
 * indicates the similarity between the muted nn and nn before mutation
 * in practice, this function adjusts the weights and biases according to similarity
 */
void NeuralNet_Mutate(NeuralNet_t *nn, double similarity);



#endif /* CJSAI_NETWORK_H */

