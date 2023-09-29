

#include <raylib.h>
#include <raygui.h>

#include <math.h>
#include <stdlib.h>
#include <string.h> /* strlen */
#include <time.h>

#include "include/common.h"
#include "include/config.h"
#include "include/cai.h"
#include "include/road.h"
#include "include/car.h"
#include "include/utils.h"
#include "include/mem.h"


static Font s_font = { 0 };

static Road_t s_road = { 0 };
static Car_t s_traffic[DEF_TRAFFIC_COUNT] = { 0 };
static int s_trafficlanes[DEF_TRAFFIC_COUNT] = { 0 };
static int s_traffic_count = DEF_TRAFFIC_COUNT;

static Car_t s_car[1] = { 0 };
static int s_car_count = STATIC_ARRSIZE(s_car);
static Car_t *s_bestcar = NULL;


static int s_height = DEF_WIN_HEIGHT;
static int s_width = DEF_WIN_WIDTH;




static Car_t default_traffic(Car_t *car, int lane, int rely);
static void update_traffic(double traveled, double delta_time);
static void draw_traffic(Color color);

static double update_cars(double delta_time);
static double find_bestcar(void);
static void draw_cars(Color color);


static void reset(double coefficient);

static int random_lane_index(void);
static double random_lane(int *outlane);
static void draw_info(void);
static void value_box(const char *title, int x, int y, int value);



void CAI_Init(void)
{
    srand(time(NULL));

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN);
    InitWindow(DEF_WIN_WIDTH, DEF_WIN_HEIGHT, "AI in C");
    SetTargetFPS(240);

    s_font = LoadFont(DEF_FONT_FILE);
    GuiSetFont(s_font);
    GuiSetStyle(DEFAULT, TEXT_SIZE, DEF_FONT_SIZE);


    s_road = Road_Init(DEF_WIN_WIDTH / 2, 
        DEF_ROAD_WIDTH, 
        DEF_WIN_HEIGHT,
        DEF_ROAD_LANES
    );


    CAI_ASSERT(s_road.numlanes > 1, "road must have more than 1 lane\n");
    for (int i = 0; i < s_traffic_count; i++)
    {
        s_trafficlanes[i] = random_lane_index();
        double rely = TRAFFIC_DESPAWN - (int)(i / (s_road.numlanes - 1)) * TRAFFIC_NEXT;
        s_traffic[i] = default_traffic(&s_traffic[i], 
            s_trafficlanes[i], 
            rely
        );
    }

    for (int i = 0; i < s_car_count; i++)
    {
        s_car[i] = Car_Init(
            &s_car[i],
            DEF_CAR_RECT(
                DEF_WIN_WIDTH*0.5, DEF_WIN_HEIGHT*0.8), 
            CAR_HUMAN
        );
    }
    s_bestcar = &s_car[0];
}


bool CAI_IsRunning(void)
{
    if (WindowShouldClose())
        return false;

    return true;
}


void CAI_Run(void)
{
    static flt_t total_dist = 0;
    static float coefficient = 0;
    double delta_time = GetFrameTime();

    /* logic code */
    PollInputEvents();
    if (IsWindowState(FLAG_WINDOW_RESIZABLE))
    {
        int new_w = GetRenderWidth();
        int new_h = GetRenderHeight();
        double xscale = (double)new_w / s_width;
        double yscale = (double)new_h / s_height;
        s_width = new_w;
        s_height = new_h;

        for (int i = 0; i < s_car_count; i++)
        {
            Car_Reposition(&s_car[i], xscale, yscale);
        }
        for (int i = 0; i < s_traffic_count; i++)
        {
            Car_Reposition(&s_traffic[i], xscale, yscale);
        }
        Road_Recenter(&s_road, s_width / 2, s_height);
    }

    GuiSlider(DEF_VALUEBOX(s_width*0.8, s_height*0.8), "0%", "100%", &coefficient, 0, 1);
    if (GuiButton(DEF_VALUEBOX(s_width*0.8, s_height*0.9), "Mutate"))
        reset(coefficient);



    double traveled = update_cars(delta_time);
    update_traffic(traveled, delta_time);

    total_dist += traveled;
    if (total_dist >= s_road.dash_len*2)
        total_dist -= s_road.dash_len*2;



    /* rendering code */
    BeginDrawing();
        ClearBackground(RAYWHITE);
        Road_Draw(s_road, 0, s_width, total_dist);
        draw_traffic(DEF_TRAFFIC_COLOR);
        draw_cars(DEF_CAR_COLOR);
        draw_info();
    EndDrawing();
}


void CAI_Deinit(void)
{
    for (int i = 0; i < s_car_count; i++)
    {
        Car_Deinit(&s_car[i]);
    }
    for (int i = 0; i < s_traffic_count; i++)
    {
        Car_Deinit(&s_traffic[i]);
    }
    Road_Deinit(&s_road);
    UnloadFont(s_font);
    CloseWindow();
}






static Car_t default_traffic(Car_t *traffic, int lane, int rely)
{
    *traffic = Car_Init(
        traffic,
        DEF_CAR_RECT(
            Road_CenterOfLane(s_road, lane), 
            rely * DEF_WIN_HEIGHT
        ), 
        CAR_DUMMY
    );
    traffic->speed = DEF_TRAFFIC_SPD;
    return *traffic;
}

static void update_traffic(double traveled, double delta_time)
{
    for (int i = 0; i < s_traffic_count; i++)
    {
        double y = -s_traffic[i].speed * delta_time * DISTANCE_FACTOR;
        s_traffic[i].y += y + traveled;
        s_traffic[i].x = (double)Road_CenterOfLane(s_road, s_trafficlanes[i]);

        if (s_traffic[i].y < TRAFFIC_DESPAWN * s_height)
        {
            s_traffic[i].y = TRAFFIC_SPAWN * s_height;
            s_traffic[i].x = random_lane(&s_trafficlanes[i]);
        }
        else if (s_traffic[i].y > TRAFFIC_SPAWN * s_height)
        {
            s_traffic[i].y = TRAFFIC_DESPAWN * s_height;
            s_traffic[i].x = random_lane(&s_trafficlanes[i]);
        }
    }
}


static void draw_traffic(Color color)
{
    for (int i = 0; i < s_traffic_count; i++)
    {
        Car_Draw(s_traffic[i], color, false, false);
    }
}









static double update_cars(double delta_time)
{
    find_bestcar();
    double bestcar_moved = Car_UpdateYpos(s_bestcar, delta_time, 0);
    Car_UpdateXpos(s_bestcar, delta_time, 0);
    s_bestcar->y = s_height*0.8;


    for (int i = 0; i < s_car_count; i++)
    {
        Car_UpdateXpos(&s_car[i], delta_time, 0);
        if (&s_car[i] != s_bestcar)
        {
            Car_UpdateYpos(&s_car[i], delta_time, bestcar_moved);
        }


        Car_UpdateSensor(&s_car[i], s_road, s_traffic, s_traffic_count);
        if (!s_car[i].damaged)
        {
            Car_ApplyFriction(&s_car[i], delta_time);
            Car_UpdateSpeed(&s_car[i], delta_time);
            Car_UpdateControls(&s_car[i]);
            Car_CheckCollision(&s_car[i], s_road, s_traffic, s_traffic_count);
        }
    }

    return bestcar_moved;
}


static double find_bestcar(void)
{
    double move_by = 0;
    for (int i = 0; i < s_car_count; i++)
    {
        if (s_bestcar->y > s_car[i].y)
        {
            move_by = s_car[i].y - s_height*0.8;
            s_bestcar = &s_car[i];
        }
    }
    return move_by;
}


static void draw_cars(Color color)
{
    Color transparent = color;
    transparent.a = 0x30;
    for (int i = 0; i < s_car_count; i++)
    {
        Car_Draw(s_car[i], transparent, false, false);
    }

    Car_Draw(*s_bestcar, color, true, true);
}




static void reset(double coefficient)
{
    for (int i = 0; i < s_car_count; i++)
    {
        s_car[i].damaged = false;
        NeuralNet_Mutate(&s_car[i].brain, coefficient);
    }
}




static int random_lane_index(void)
{
    return utils_randflt(0, s_road.numlanes);
}


static double random_lane(int *outlane)
{
    *outlane = random_lane_index();
    return (double)Road_CenterOfLane(s_road, *outlane) / s_width;
}



static void value_box(const char *title, int x, int y, int val)
{
    GuiValueBox(
        (Rectangle){
            .x = x, .y = y, 
            .width = DEF_VALUEBOX_WIDTH, 
            .height = DEF_VALUEBOX_HEIGHT
        }, 
        title, &val, 0, val, false
    );
}


static void draw_info(void)
{
    int x = 60, y = 50;
    value_box("fps:", x, y, GetFPS());
    y += DEF_VALUEBOX_HEIGHT;
    value_box("mph:", x, y, s_bestcar->speed / 1.6);

#ifdef _DEBUG
    y += DEF_VALUEBOX_HEIGHT;
    value_box("angle:", x, y, s_bestcar->angle);

    y += DEF_VALUEBOX_HEIGHT;
    value_box("xpos:", x, y, s_bestcar->x);

    y += DEF_VALUEBOX_HEIGHT;
    value_box("ypos:", x, y, s_bestcar->y);

    y += DEF_VALUEBOX_HEIGHT;
    value_box("dmg: ", x, y, s_bestcar->damaged);

    y += DEF_VALUEBOX_HEIGHT;
    value_box("rleft: ", x + 30, y, s_road.left);
    y += DEF_VALUEBOX_HEIGHT;
    value_box("rright:", x + 30, y, s_road.right);


    static char tmp[256] = { 0 };
    for (int i = 0; i < s_traffic_count; i++)
    {
        y += DEF_VALUEBOX_HEIGHT;
        snprintf(tmp, sizeof tmp, "traffic %d: y:", i);
        value_box(tmp, x*2, y, s_traffic[i].y);
    }
#endif /* _DEBUG */
}
