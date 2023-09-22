

#include <raylib.h>
#include <raygui.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "include/common.h"
#include "include/config.h"
#include "include/cai.h"
#include "include/road.h"
#include "include/car.h"
#include "include/utils.h"


static Font s_font = { 0 };

static Road_t s_road = { 0 };
static Car_t s_traffic[DEF_TRAFFIC_COUNT] = { 0 };
static int s_trafficlanes[DEF_TRAFFIC_COUNT] = { 0 };
static int s_traffic_count = DEF_TRAFFIC_COUNT;

static Car_t s_car = { 0 };
static Car_t *s_focused_car = NULL;


static Car_t default_traffic(int lane, int y);
static void update_traffic(int win_w, int win_h, double traveled, double delta_time);
static void draw_traffic(int win_w, int win_h);

static double update_car(int win_w, int win_h, double delta_time);
static void update_car_direction(double delta_time);
/* apply friction to the focused car */
static double apply_friction(double delta_time);
static double random_lane(int win_w, int *outlane);
static void draw_info(void);



void CAI_Init(void)
{
    srand(time(NULL));

    InitWindow(DEF_WIN_WIDTH, DEF_WIN_HEIGHT, "AI in C");
    SetWindowState(FLAG_WINDOW_ALWAYS_RUN);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(240);

    s_font = LoadFont(DEF_FONT_FILE);
    GuiSetFont(s_font);
    GuiSetStyle(DEFAULT, TEXT_SIZE, DEF_FONT_SIZE);


    s_road = Road_Init(DEF_WIN_WIDTH / 2, 
        DEF_ROAD_WIDTH, 
        DEF_ROAD_LANES
    );


    s_traffic[0] = default_traffic(0, -DEF_CAR_WIDTH);
    s_traffic[1] = default_traffic(1, -DEF_CAR_WIDTH);
    s_trafficlanes[0] = 0;
    s_trafficlanes[1] = 1;

    s_car = Car_Init(
        DEF_CAR_RECT(0.5, 0.8), 
        DEF_CAR_COLOR,
        CAR_HUMAN
    );
    s_focused_car = &s_car;
}


bool CAI_IsRunning(void)
{
    if (WindowShouldClose())
        return false;

    PollInputEvents();
    return true;
}


void CAI_Run(void)
{
    static float total_dist = 0; 
    int win_h = GetRenderHeight();
    int win_w = GetRenderWidth();
    double delta_time = GetFrameTime();

    /* logic code */
    Road_Recenter(&s_road, win_w / 2);
    double traveled = update_car(win_w, win_h, delta_time);
    update_traffic(win_w, win_h, traveled, delta_time);

    total_dist += traveled;
    if (total_dist >= s_road.dash_len*2)
        total_dist -= s_road.dash_len*2;



    /* rendering code */
    BeginDrawing();
        ClearBackground(RAYWHITE);
        Road_Draw(s_road, 0, win_h, total_dist);
        Car_Draw(s_car, win_w, win_h);
        draw_traffic(win_w, win_h);
        draw_info();
    EndDrawing();
}


void CAI_Deinit(void)
{
    Car_Deinit(&s_car);
    for (int i = 0; i < s_traffic_count; i++)
    {
        Car_Deinit(&s_traffic[i]);
    }
    Road_Deinit(&s_road);
    UnloadFont(s_font);
    CloseWindow();
}






static Car_t default_traffic(int lane, int y)
{
    Car_t car = Car_Init(
        DEF_CAR_RECT(
            Road_CenterOfLane(s_road, lane) / (double)DEF_WIN_WIDTH, 
            (double)y / DEF_WIN_HEIGHT), 
        DEF_TRAFFIC_COLOR, 
        CAR_DUMMY
    );
    car.speed = DEF_TRAFFIC_SPD;
    return car;
}

static void update_traffic(int win_w, int win_h, double traveled, double delta_time)
{
    for (int i = 0; i < s_traffic_count; i++)
    {
        double y = -s_traffic[i].speed * delta_time * DISTANCE_FACTOR;
        s_traffic[i].rely += (y + traveled) / win_h;
        s_traffic[i].relx = (double)Road_CenterOfLane(s_road, s_trafficlanes[i]) / win_w;

        CAI_DEBUG_PRINT("traffic %d: rely: %.02f\n", i, s_traffic[i].rely);

        if (s_traffic[i].rely < -.2)
        {
            s_traffic[i].rely = 1.1;
            s_traffic[i].relx = random_lane(win_w, &s_trafficlanes[i]);
        }
        else if (s_traffic[i].rely > 1.5)
        {
            s_traffic[i].rely = -.1;
            s_traffic[i].relx = random_lane(win_w, &s_trafficlanes[i]);
        }
    }
}


static void draw_traffic(int win_w, int win_h)
{
    for (int i = 0; i < s_traffic_count; i++)
    {
        Car_Draw(s_traffic[i], win_w, win_h);
    }
}



static double update_car(int win_w, int win_h, double delta_time)
{
    apply_friction(delta_time);


    if (!s_focused_car->damaged)
    {
        update_car_direction(delta_time);
    }

    double traveled = s_focused_car->speed * delta_time * DISTANCE_FACTOR;
    s_focused_car->relx += sin(DEG_TORAD(s_focused_car->angle)) * traveled / win_w;


    CAI_DEBUG_PRINT("km/h: %.02f, mph: %.02f, angle: %.02f, ",
        s_focused_car->speed, 
        s_focused_car->speed / 1.6f,
        s_focused_car->angle
    );
    CAI_DEBUG_PRINT("w: %d, h: %d, carx: %.2f, cary: %.2f\n",
        win_w, win_h, 
        s_focused_car->relx * win_w, s_focused_car->rely * win_h
    );

    /* returns distance traveled in the y direction */
    return cos(DEG_TORAD(s_focused_car->angle)) * traveled;
}



static double apply_friction(double delta_time)
{
    return s_focused_car->speed -= s_focused_car->speed * DEF_CAR_FRICTION * delta_time;
}


static void update_car_direction(double delta_time)
{    
    /* turning */
    double angle = DEF_CAR_ANGLETURN;
    if (flt_inrange(-DEF_CAR_REVERSESPD, s_focused_car->speed, 20))
        angle *= delta_time;
    else 
        angle /= s_focused_car->speed / 2;
    if (IsKeyDown(KEY_A))
    {
        s_focused_car->angle -= angle;
        apply_friction(delta_time);
    }
    else if (IsKeyDown(KEY_D))
    {
        s_focused_car->angle += angle;
        apply_friction(delta_time);
    }


    /* forward and reverse */
    if (IsKeyDown(KEY_W))
    {
        s_focused_car->speed += DEF_CAR_ACCEL * delta_time;
    }
    if (s_focused_car->speed > -DEF_CAR_REVERSESPD)
    {
        if (IsKeyDown(KEY_S))
            s_focused_car->speed -= DEF_CAR_DECEL * delta_time;
    }
}




static double random_lane(int win_w, int *outlane)
{
    *outlane = utils_randflt(0, s_road.numlanes);
    return (double)Road_CenterOfLane(s_road, *outlane) / win_w;
}


static void draw_info(void)
{
    int fps = GetFPS();
    GuiValueBox((Rectangle){.x = 50, .y = 50, .width = 40, .height = 20}, 
        "fps:", &fps, 0, fps, false
    );

    int spd = s_focused_car->speed / 1.6;
    GuiValueBox((Rectangle){.x = 50, .y = 70, .width = 40, .height = 20},
        "mph:", &spd, 0, spd, false
    );
}
