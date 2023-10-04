

#include <raylib.h>

#include "include/road.h"
#include "include/utils.h"
#include "include/config.h"




Road_t Road_Init(Road_t *road, flt_t center, flt_t w, flt_t h, int numlanes)
{
    flt_t x = center - w/2;
    *road = (Road_t){
        .width = w,
        .height = h,
        .numlanes = numlanes,
        .color = DEF_ROAD_COLOR,
        .dash_len = DEF_ROAD_DASHLEN,
        .dash_width = DEF_ROAD_DASHWIDTH,
        .dash_color = DEF_ROAD_DASHCOLOR,

        .points[0] = {.x = x, .y = 0},
        .points[1] = {.x = x, .y = -h},
        .points[2] = {.x = x + w, .y = -h},
        .points[3] = {.x = x + w, .y = 0},
    };
    road->lane_width = (w - 4*road->dash_width) / numlanes;
    road->poly = Polygon_Init(STATIC_ARRSIZE(road->points), road->points);
    return *road;
}


void Road_Deinit(Road_t *road)
{
    *road = ZEROALL(Road_t);
}






void Road_Recenter(Road_t *road, flt_t width, flt_t h)
{
    flt_t x = (width - road->width) / 2;
    road->height = h;
    road->points[0] = (Vector2){.x = x, .y = 0};
    road->points[1] = (Vector2){.x = x, .y = h};
    road->points[2] = (Vector2){.x = x + road->width, .y = h};
    road->points[3] = (Vector2){.x = x + road->width, .y = 0};
}


double Road_GetCenter(const Road_t road)
{
    return road.points[0].x + (int)(road.width / 2);
}



Line_t Road_RightBorder(const Road_t road)
{
    return Line_From(road.points[2].x, 0, 0, road.height);
}


Line_t Road_LeftBorder(const Road_t road)
{
    return Line_From(road.points[0].x, 0, 0, road.height);
}



void Road_Draw(const Road_t road, int y_start, int y_end, int divider_offset)
{
    /* the surface */
    DrawRectangleV(road.points[0], 
        (Vector2){.x = road.width, .y = road.height}, 
        road.color
    );

    if (road.numlanes < 2)
        return;

    /* lane dash/divider */
    int dash_start = y_start + divider_offset - road.dash_len;
    int x = road.points[0].x + 2*road.dash_width;
    for (int i = 0; i < road.numlanes - 1; i++)
    {
        x += road.lane_width;
        for (int y = dash_start; y < y_end; y += 2 * road.dash_len)
        {
            DrawRectangle(
                x - road.dash_width / 2, y, 
                road.dash_width, road.dash_len, 
                road.dash_color
            );
        }
    }

    /* road edge */
    int left = road.points[0].x;
    int right = left + road.width;
    DrawRectangle(
        left + road.dash_width, y_start, 
        road.dash_width, y_end, 
        road.dash_color
    );
    DrawRectangle(
        right - 2*road.dash_width, y_start, 
        road.dash_width, y_end, 
        road.dash_color
    );
}


int Road_CenterOfLane(const Road_t road, int lane)
{
    CAI_ASSERT(lane < road.numlanes, 
        "Invalid input: %d, expect < %d\n", lane, road.numlanes
    );

    const int center = road.lane_width / 2;
    return road.points[0].x + 2*road.dash_width + lane * road.lane_width + center;
}


