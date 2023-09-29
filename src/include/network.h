#ifndef CJSAI_NETWORK_H
#define CJSAI_NETWORK_H


#include "common.h"
#include "level.h"


typedef struct NeuralNet_t
{
    Level_t *levels;
    usize_t count;
} NeuralNet_t;


/* each elems in level contains the number of neuron that level has
 *  level: {2, 1, 3}, neuron_count: 3
 *  NOTE: neuron_count >= 2
 *
 *  there will be 2 levels in this network, where
 *      level 0: in: 2, out: 1
 *      level 1: in: 1, out: 3
 */
NeuralNet_t NeuralNet_Init(usize_t *level, usize_t neuron_count);
void NeuralNet_Deinit(NeuralNet_t *nn);

/* a wrapper around level's feed forward,
 * assumes that the input field of nn->levels[0] is already poppulated 
 */
bitarr_t NeuralNet_FeedForward(NeuralNet_t *nn);


/* 0 <= similarity <= 1,
 * indicates the similarity between the muted nn and nn before mutation
 * in practice, this function adjusts the weights and biases according to similarity
 */
void NeuralNet_Mutate(NeuralNet_t *nn, double similarity);



#endif /* CJSAI_NETWORK_H */

