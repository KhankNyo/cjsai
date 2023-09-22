#ifndef CAI_ROAD_H
#define CAI_ROAD_H



#include <raylib.h>

typedef struct Road_t
{
    int left, right;
    int width, lane_width;
    int numlanes;
    int dash_len, dash_width;
    Color dash_color, color;
} Road_t;

Road_t Road_Init(int center, int width, int numlanes);
void Road_Deinit(Road_t *road);


void Road_Recenter(Road_t *road, int center);

/* draws whole road */
void Road_Draw(const Road_t, int y_start, int y_end, int divider_offset);

/* lane is zero indexed, 
 * returns the absolute position of the center of a lane */
int Road_CenterOfLane(const Road_t, int lane);



#endif /* CAI_ROAD_H */

