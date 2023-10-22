
#include <time.h> /* time */
#include <string.h> /* memset */

#include "include/level.h"
#include "include/mem.h"
#include "include/config.h"



static void randomize(Level_t *level);






Level_t Level_Init(usize_t input_count, usize_t output_count, bool initialize)
{
    CAI_ASSERT(input_count < BITARR_COUNT, 
        "a level's input of a level must be < %d\n", BITARR_COUNT
    );
    CAI_ASSERT(output_count < BITARR_COUNT, 
        "a number of output of a level must be < %d\n", BITARR_COUNT
    );


    Level_t level = {
        .inputs = fltarr_Init(),
        .outputs = bitarr_Init(),
        .output_count = output_count,
        .biases = fltarr_Init(),

        .weights = MEM_ALLOC_ARRAY(
            input_count, sizeof(level.weights[0])
        ),
        .weight_count = input_count,
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

    if (initialize)
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




Level_t Level_Copy(Level_t *dst, const Level_t src)
{
    Level_t l;
    if (NULL == dst)
    {
        l = Level_Init(src.inputs.count, src.output_count, true);
        dst = &l;
    }


    if (dst->weight_count < src.weight_count)
    {
        dst->weights = MEM_REALLOC_ARRAY(dst->weights, 
            src.inputs.count, sizeof(dst->weights[0])
        );
        for (usize_t i = dst->inputs.count; i < src.inputs.count; i++)
            dst->weights[i] = fltarr_Copy(NULL, src.weights[i]);
    }

    usize_t max_inputcount = dst->inputs.count < src.inputs.count 
        ? src.inputs.count
        : dst->inputs.count;
    for (usize_t i = 0; i < max_inputcount; i++)
        fltarr_Copy(&dst->weights[i], src.weights[i]);

    fltarr_Copy(&dst->inputs, src.inputs);
    fltarr_Copy(&dst->biases, src.biases);
    bitarr_Copy(&dst->outputs, src.outputs);
    dst->output_count = src.output_count;
    return *dst;
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






void Level_Draw(const Level_t level, Rectangle bound, flt_t node_radius, bool draw_input)
{
    flt_t bufdist = 5;
    flt_t in_node_dist = bound.width / (level.inputs.count);
    flt_t out_node_dist = bound.width / (level.output_count);
    flt_t radius = node_radius - 4;

    flt_t line_thickness = 2;

    Vector2 out = {
        .x = bound.x + radius + bufdist, 
        .y = bound.y + radius + bufdist
    };
    Vector2 in = {
        .x = bound.x + radius + bufdist, 
        .y = bound.y + bound.height - radius - bufdist
    };

    Color positive_color = GREEN;
    Color negative_color = RED;


    Vector2 center = {.x = in.x, .y = in.y};
    if (draw_input)
    {
        for (int i = (int)level.inputs.count - 1; i >= 0; i--)
        {
            Color color = positive_color;
            flt_t activation = level.inputs.at[i];
            if (activation < 0)
            {
                color = negative_color;
                activation = -activation;
            }
            color.a = BYTE_PERCENTAGE(activation);

            DrawCircleV(center, radius, color);
            center.x += in_node_dist;
        }
    }

    /* draw the outputs */
    center.x = out.x;
    center.y = out.y;
    for (usize_t i = 0; i < level.output_count; i++)
    {
        usize_t val = bitarr_Get(level.outputs, i);
        Color color = val ? positive_color : negative_color;
        DrawCircleV(center, radius, color);
        center.x += out_node_dist;
    }


    /* draw the lines */
    flt_t in_xstart = in.x;
    for (usize_t i = 0; i < level.output_count; i++, 
        out.x += out_node_dist, in.x = in_xstart)
    {
        in.x = in_xstart;
        for (usize_t k = 0; k < level.inputs.count; k++, 
            in.x += in_node_dist)
        {
            Color weight = positive_color;
            flt_t scalar = level.weights[k].at[i];
            if (scalar < 0)
            {
                weight = negative_color;
                scalar = -scalar;
            }
            weight.a = BYTE_PERCENTAGE(scalar);

            DrawLineEx(out, in, line_thickness, weight);
        }
    }
}










static void randomize(Level_t *level)
{
    for (usize_t i = 0; i < level->inputs.count; i++)
    {
        for (usize_t j = 0; j < level->weights[i].count; j++)
        {
            level->weights[i].at[j] = utils_randflt(-1, 1);
        }
    }

    for (usize_t i = 0; i < level->biases.count; i++)
    {
        level->biases.at[i] = utils_randflt(-1, 1);
    }
}


