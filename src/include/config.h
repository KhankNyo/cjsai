#ifndef CAI_CONFIG_H
#define CAI_CONFIG_H


#include <math.h>
#include <raylib.h>


#define DEF_FONT_FILE "resources/font/cascadia.ttf"
#define DEF_FONT_SIZE 16

#define DEF_WIN_WIDTH 1080
#define DEF_WIN_HEIGHT 720


#define DEF_VALUEBOX_HEIGHT 20 
#define DEF_VALUEBOX_WIDTH 50
#define DEF_VALUEBOX(argx, argy) (Rectangle){\
    .x = argx, .y = argy,\
    .width = DEF_VALUEBOX_WIDTH, .height = DEF_VALUEBOX_HEIGHT}

#define DEF_ROAD_WIDTH ((double)DEF_WIN_WIDTH / 4)
#define DEF_ROAD_DASHLEN 30
#define DEF_ROAD_DASHWIDTH 5
#define DEF_ROAD_LANES 3
#define DEF_ROAD_COLOR GRAY
#define DEF_ROAD_DASHCOLOR RAYWHITE
#define DEF_ROAD_LANEWIDTH ((double)DEF_ROAD_WIDTH / (double)DEF_ROAD_LANES - DEF_ROAD_DASHWIDTH)



#define DISTANCE_FACTOR 10
#define DEF_ASPECT_RATIO ((double)DEF_WIN_HEIGHT / DEF_WIN_WIDTH)

#define DEF_TRAFFIC_COUNT 4
#define DEF_TRAFFIC_SPD (25*1.6f) /* 30 mph */
#define DEF_TRAFFIC_COLOR RED
#define TRAFFIC_BOTTOM (2.f * DEF_ASPECT_RATIO)
#define TRAFFIC_TOP (-2.f * DEF_ASPECT_RATIO)
#define TRAFFIC_NEXT -(((double)DEF_ROAD_LANES - 1) / DEF_TRAFFIC_COUNT)


#define DEF_CAR_FRICTION .090f
#define DEF_CAR_ANGLETURN 180.0f
#define DEF_CAR_ACCEL 25.0f
#define DEF_CAR_DECEL (DEF_CAR_ACCEL * 3)
#define DEF_CAR_REVERSESPD 0.0f
#define DEF_CAR_TOPSPD (40*1.6)

#define DEF_CAR_COLOR DARKBLUE

#define DEF_CAR_WIDTH ((double)DEF_ROAD_LANEWIDTH * 5/8)
#define DEF_CAR_LEN (DEF_CAR_WIDTH * 7/4)

#define DEF_CAR_RECT(arg_x, arg_y) \
    (Rectangle){.x = arg_x, .y = arg_y, \
        .width = DEF_CAR_WIDTH, .height = DEF_CAR_LEN}
#define DEF_CAR_POLYCOUNT 4



#define DEF_SENSOR_RAYLEN (200.0f)
#define DEF_SENSOR_RAYSPREAD (70)
#define DEF_SENSOR_COLORTOUCHED BLACK
#define DEF_SENSOR_COLORNORMAL YELLOW
#define DEF_SENSOR_RAYCOUNT 7


#define DEF_NN_ARCHITECTURE (usize_t[2]){DEF_SENSOR_RAYCOUNT + 1, 4}
#define DEF_NN_SIZE 2


#endif /* CAI_CONFIG_H */

