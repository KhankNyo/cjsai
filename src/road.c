

#include <raylib.h>

#include "include/road.h"
#include "include/utils.h"
#include "include/config.h"




Road_t Road_Init(int center, int width, int numlanes)
{
    Road_t road = {
        .width = width, 
        .lane_width = width / numlanes,
        .numlanes = numlanes,
        .color = DEF_ROAD_COLOR,

        .left = center - width / 2,
        .right = center + width / 2,

        .dash_len = DEF_ROAD_DASHLEN,
        .dash_width = DEF_ROAD_DASHWIDTH,
        .dash_color = DEF_ROAD_DASHCOLOR,
    };
    return road;
}


void Road_Deinit(Road_t *road)
{
    *road = UTILS_ZEROALL(Road_t);
}






void Road_Recenter(Road_t *road, int center)
{
    road->left = center - road->width / 2;
    road->right = center + road->width / 2;
}


void Road_Draw(const Road_t road, int y_start, int y_end, int divider_offset)
{
    /* the surface */
    DrawRectangle(
        road.left, y_start, 
        road.width, y_end, 
        road.color
    );

    /* lane dash/divider */
    y_start += divider_offset - road.dash_len;
    for (int x = road.left + road.lane_width; 
        x < road.right; 
        x += road.lane_width)
    {
        for (int y = y_start; y < y_end; y += 2 * road.dash_len)
        {
            DrawRectangle(
                x - road.dash_width / 2, y, 
                road.dash_width, road.dash_len, 
                road.dash_color
            );
        }
    }
}


int Road_CenterOfLane(const Road_t road, int lane)
{
    CAI_ASSERT(lane < road.numlanes, 
        "Invalid input: %d, expect < %d\n", lane, road.numlanes
    );

    const int center = road.lane_width / 2;
    return road.left + lane * road.lane_width + center;
}


