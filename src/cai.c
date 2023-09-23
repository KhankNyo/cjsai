

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


static Car_t default_traffic(int lane, int rely);
static void update_traffic(int win_w, int win_h, double traveled, double delta_time);
static void draw_traffic(int win_w, int win_h);

static double update_cars(int win_w, int win_h, double delta_time);

static int random_lane_index(void);
static double random_lane(int win_w, int *outlane);
static void draw_info(void);
static void value_box(const char *title, int x, int y, int value);



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


    CAI_ASSERT(s_road.numlanes > 1, "road must have more than 1 lane\n");
    for (int i = 0; i < s_traffic_count; i++)
    {
        s_trafficlanes[i] = random_lane_index();
        double rely = TRAFFIC_DESPAWN - (int)(i / (s_road.numlanes - 1)) * TRAFFIC_NEXT;
        s_traffic[i] = default_traffic(s_trafficlanes[i], rely);
    }

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
    static flt_t total_dist = 0; 
    int win_h = GetRenderHeight();
    int win_w = GetRenderWidth();
    double delta_time = GetFrameTime();

    /* logic code */
    Road_Recenter(&s_road, win_w / 2);
    double traveled = update_cars(win_w, win_h, delta_time);
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






static Car_t default_traffic(int lane, int rely)
{
    Car_t car = Car_Init(
        DEF_CAR_RECT(
            Road_CenterOfLane(s_road, lane) / (double)DEF_WIN_WIDTH, 
            rely
        ), 
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

        if (s_traffic[i].rely < TRAFFIC_DESPAWN)
        {
            s_traffic[i].rely = TRAFFIC_SPAWN;
            s_traffic[i].relx = random_lane(win_w, &s_trafficlanes[i]);
        }
        else if (s_traffic[i].rely > TRAFFIC_SPAWN)
        {
            s_traffic[i].rely = TRAFFIC_DESPAWN;
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









static double update_cars(int win_w, int win_h, double delta_time)
{
    Car_ApplyFriction(s_focused_car, delta_time);
    Car_UpdateControls(s_focused_car, delta_time);
    Car_UpdateXpos(s_focused_car, win_w, delta_time);

    double rely = s_focused_car->rely;
    double dist = Car_UpdateYpos(s_focused_car, win_h, delta_time);
    s_focused_car->rely = rely;
    return dist;
}




static int random_lane_index(void)
{
    return utils_randflt(0, s_road.numlanes);
}


static double random_lane(int win_w, int *outlane)
{
    *outlane = random_lane_index();
    return (double)Road_CenterOfLane(s_road, *outlane) / win_w;
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
    value_box("mph:", x, y, s_focused_car->speed / 1.6);

#ifdef _DEBUG
    y += DEF_VALUEBOX_HEIGHT;
    value_box("angle:", x, y, s_focused_car->angle);

    y += DEF_VALUEBOX_HEIGHT;
    value_box("xpos:", x, y, s_focused_car->relx * GetRenderWidth());

    y += DEF_VALUEBOX_HEIGHT;
    value_box("ypos:", x, y, s_focused_car->rely * GetRenderHeight());


    for (int i = 0; i < s_traffic_count; i++)
    {
        static char tmp[32] = { 0 };
        y += DEF_VALUEBOX_HEIGHT;
        snprintf(tmp, sizeof tmp, "traffic %d: y:", i);
        value_box(tmp, x*2, y, s_traffic[i].rely * GetRenderHeight());
    }
#endif /* _DEBUG */
}
