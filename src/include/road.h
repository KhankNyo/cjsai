#ifndef CAI_ROAD_H
#define CAI_ROAD_H



#include <raylib.h>

#include "line.h"
#include "common.h"
#include "utils.h"



typedef struct Road_t
{
    int lane_width;
    int numlanes;
    int dash_len, dash_width;
    Color dash_color, color;

    flt_t width, height;
    Vector2 points[4];
    Polygon_t poly;
} Road_t;

Road_t Road_Init(Road_t *road, flt_t center, flt_t width, flt_t height, int numlanes);
void Road_Deinit(Road_t *road);


void Road_Recenter(Road_t *road, flt_t topleft_x, flt_t topleft_y);
double Road_GetCenter(const Road_t road);

Line_t Road_RightBorder(const Road_t road);
Line_t Road_LeftBorder(const Road_t road);

/* draws whole road */
void Road_Draw(const Road_t, int y_start, int y_end, int divider_offset);

/* lane is zero indexed, 
 * returns the absolute position of the center of a lane */
int Road_CenterOfLane(const Road_t, int lane);



#endif /* CAI_ROAD_H */

