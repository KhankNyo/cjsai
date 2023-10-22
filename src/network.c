
#include <string.h> /* memset, memcpy */
#include <raylib.h>

#include "include/network.h"
#include "include/mem.h"
#include "include/level.h"








NNArch_t NNArch_Init(usize_t count, usize_t *levels)
{
    NNArch_t arch = {
        .levels = MEM_ALLOC_ARRAY(count, sizeof(*levels)),
        .count = count,
        .capacity = count,
    };
    memcpy(arch.levels, levels, count * sizeof(*levels));
    return arch;
}


void NNArch_Deinit(NNArch_t *arch)
{
    MEM_FREE_ARRAY(arch->levels);
    memset(arch, 0, sizeof(*arch));
}


bool NNArch_Change(NNArch_t *arch, usize_t count, usize_t *levels)
{
    if (arch->capacity < count)
    {
        NNArch_Deinit(arch);
        *arch = NNArch_Init(count, levels);
        return true;
    }

    arch->count = count;
    memcpy(arch->levels, levels, count * sizeof(*levels));
    return false;
}









NeuralNet_t NeuralNet_Init(const NNArch_t arch, bool initialize)
{
    CAI_ASSERT(arch.count > 1, "There must be at least 2 neurons in a neural network\n");

    NeuralNet_t nn = {
        .levels = MEM_ALLOC_ARRAY(arch.count, sizeof(nn.levels[0])),
        .count = arch.count - 1,
    };
    for (usize_t i = 0; i < nn.count; i++)
    {
        nn.levels[i] = Level_Init(arch.levels[i], arch.levels[i + 1], initialize);
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


void NeuralNet_Mutate(NeuralNet_t *nn, const NeuralNet_t src, flt_t similarity)
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




void NeuralNet_Draw(const NeuralNet_t nn, Rectangle container, Color background, flt_t node_radius)
{
    DrawRectangleRec(container, background);

    flt_t level_height = container.height / nn.count;
    container.y += level_height * (nn.count - 1);
    container.height = level_height;
    for (usize_t i = 0; i < nn.count; i++)
    {
        Level_Draw(nn.levels[i], container, node_radius, 0 == i);
        container.y -= level_height - node_radius*2;
    }
}




