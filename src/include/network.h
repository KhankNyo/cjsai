#ifndef CJSAI_NETWORK_H
#define CJSAI_NETWORK_H


#include "common.h"
#include "level.h"


typedef struct NNArch_t
{
    usize_t *levels;
    usize_t count;
} NNArch_t;


NNArch_t NNArch_Init(usize_t count, usize_t *levels);
NNArch_t NNArch_Change(NNArch_t *arch, usize_t count, usize_t *levels);
void NNArch_Deinit(NNArch_t *arch);



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
NeuralNet_t NeuralNet_Init(const NNArch_t arch);
void NeuralNet_Deinit(NeuralNet_t *nn);
void NeuralNet_Copy(NeuralNet_t *nn, const NeuralNet_t src);

/* a wrapper around level's feed forward,
 * assumes that the input field of nn->levels[0] is already poppulated 
 */
bitarr_t NeuralNet_FeedForward(NeuralNet_t *nn);


/* 0 <= similarity <= 1,
 * indicates how similar the new dst is to src
 * in practice, this function adjusts the weights and biases according to similarity
 */
void NeuralNet_Mutate(NeuralNet_t *dst, const NeuralNet_t src, double similarity);


void NeuralNet_Draw(const NeuralNet_t nn, Rectangle container);







#endif /* CJSAI_NETWORK_H */

