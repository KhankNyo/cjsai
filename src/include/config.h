#ifndef CAI_CONFIG_H
#define CAI_CONFIG_H


#include <raylib.h>


#define DEF_FONT_FILE "resources/font/cascadia.ttf"
#define DEF_FONT_SIZE 16

#define DEF_WIN_WIDTH 1080
#define DEF_WIN_HEIGHT 720

#define DEF_ROAD_WIDTH ((double)DEF_WIN_WIDTH / 4)
#define DEF_ROAD_DASHLEN 30
#define DEF_ROAD_DASHWIDTH 5
#define DEF_ROAD_LANES 3
#define DEF_ROAD_COLOR GRAY
#define DEF_ROAD_DASHCOLOR RAYWHITE
#define DEF_ROAD_LANEWIDTH ((double)DEF_ROAD_WIDTH / (double)DEF_ROAD_LANES - DEF_ROAD_DASHWIDTH)



#define DISTANCE_FACTOR 10

#define DEF_TRAFFIC_COUNT 2
#define DEF_TRAFFIC_SPD (30*1.6f) /* 30 mph */
#define DEF_TRAFFIC_COLOR RED

#define DEF_CAR_FRICTION .090f
#define DEF_CAR_ANGLETURN 30.0f
#define DEF_CAR_ACCEL 25.0f
#define DEF_CAR_DECEL (DEF_CAR_ACCEL * 3)
#define DEF_CAR_REVERSESPD 20.0f
#define DEF_CAR_COLOR BLUE
#define DEF_CAR_WIDTH (DEF_ROAD_LANEWIDTH * 5/8)
#define DEF_CAR_RECT(arg_x, arg_y) \
    (Rectangle){.x = arg_x, .y = arg_y, \
        .width = DEF_CAR_WIDTH, .height = 75}


#endif /* CAI_CONFIG_H */

