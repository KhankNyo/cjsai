

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
static Color s_traffic_color;
static int s_trafficlanes[DEF_TRAFFIC_COUNT] = { 0 };
static int s_traffic_count = DEF_TRAFFIC_COUNT;

static Car_t s_cars[5000] = { 0 };
static Color s_car_color;
static int s_car_count = STATIC_ARRSIZE(s_cars);


static Car_t *s_bestcar = NULL;



static int s_height = DEF_WIN_HEIGHT;
#define BESTCAR_YPOS() (s_height*0.8)
static int s_width = DEF_WIN_WIDTH;
static int s_updated = 0;



#define LIST
#define STATIC_LIST

typedef struct CarList_t
{
    Car_t *car;
    struct CarList_t *next;
} CarList_t;
#ifdef STATIC_LIST
    static CarList_t s_pool[STATIC_ARRSIZE(s_cars)];
    static CarList_t *s_stack[STATIC_ARRSIZE(s_cars)];
#else
    static CarList_t *s_pool = NULL;
    static CarList_t **s_stack = NULL;
#endif /* STATIC_LIST */
static int s_poolsize = 0;
static CarList_t **s_sp = NULL;
#define PUSH(p_carlist) (*(--s_sp) = (p_carlist))
#define POP() (*(s_sp++))

static CarList_t *s_head = NULL;
static void pool_init(int elems);
static void pool_deinit(void);
static void list_init(void);
static void list_append(Car_t *car);
static void list_remove(CarList_t *last, CarList_t *node);





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

static bool is_killable(const Car_t *car);
static bool better_than_bestcar(const Car_t *car);





void CAI_Init(void)
{
    pool_init(s_car_count);
    list_init();

    s_car_color = DEF_CAR_COLOR;
    s_traffic_color = DEF_TRAFFIC_COLOR;
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
        double rely = TRAFFIC_TOP - (int)(i / (s_road.numlanes - 1)) * TRAFFIC_NEXT;
        s_traffic[i] = default_traffic(&s_traffic[i], 
            s_trafficlanes[i], 
            rely
        );
    }

    for (int i = 0; i < s_car_count; i++)
    {
        s_cars[i] = Car_Init(
            &s_cars[i],
            DEF_CAR_RECT(
                DEF_WIN_WIDTH*0.5, BESTCAR_YPOS()), 
            CAR_AI
        );

    }
    s_bestcar = &s_cars[0];
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
    static float coefficient = 1;
    double delta_time = GetFrameTime();

    /* logic code */
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
            Car_Reposition(&s_cars[i], xscale, yscale);
        }
        for (int i = 0; i < s_traffic_count; i++)
        {
            Car_Reposition(&s_traffic[i], xscale, yscale);
        }
        Road_Recenter(&s_road, s_width / 2, s_height);
    }

    GuiSlider(DEF_VALUEBOX(s_width*0.8, s_height*0.8), "0%", "100%", &coefficient, 0, 1);
    value_box("mutation:", s_width*0.8, s_height*0.85, coefficient * 100);
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
        draw_traffic(s_traffic_color);
        draw_cars(s_car_color);
        draw_info();
    EndDrawing();
}


void CAI_Deinit(void)
{
    for (int i = 0; i < s_car_count; i++)
    {
        Car_Deinit(&s_cars[i]);
    }
    for (int i = 0; i < s_traffic_count; i++)
    {
        Car_Deinit(&s_traffic[i]);
    }
    pool_deinit();
    Road_Deinit(&s_road);
    UnloadFont(s_font);
    CloseWindow();
}









static void pool_init(int elems)
{
#ifndef STATIC_LIST
    s_pool = MEM_ALLOC_ARRAY(elems, sizeof(s_pool[0]));
    s_stack = MEM_ALLOC_ARRAY(elems, sizeof(s_stack[0]));
#endif /* STATIC_LIST */
    s_poolsize = elems;
}


static void pool_deinit(void)
{
#ifndef STATIC_LIST
    mem_free(s_pool);
    mem_free(s_stack);

    s_pool = NULL;
    s_poolsize = 0;
    s_stack = NULL;
#endif /* STATIC_LIST */
}


static void list_init(void)
{
    CAI_ASSERT(NULL != s_pool, "must call pool_init before list_init\n");
    CAI_ASSERT(0 != s_poolsize, "pool size must be > 0\n");

    s_sp = &s_stack[0];
    s_head = NULL;
    for (int i = 0; i < s_poolsize; i++)
    {
        s_stack[i] = &s_pool[i];
        list_append(&s_cars[i]);
    }

    CAI_ASSERT(s_head != NULL, "head must not be null");
}


static void list_append(Car_t *car)
{
    CarList_t *node = POP();
    node->car = car;
    node->next = s_head;
    s_head = node;
}


static void list_remove(CarList_t *last, CarList_t *node)
{
    if (NULL != last)
        last->next = node->next;
    else 
        s_head = s_head->next;
    node->next = NULL;
    node->car = NULL;
    PUSH(node);
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


static void respawn_traffic(int i, double spawn)
{
    static int lane = 0;
    static bool spawnleft = true;

    s_trafficlanes[i] = lane;
    s_traffic[i].y = spawn;
    s_traffic[i].x = Road_CenterOfLane(s_road, lane);

    if (spawnleft && lane < s_road.numlanes - 1)
    {
        lane++;
    }
    else if (lane > 1) 
    {
        spawnleft = false;
        lane--;
    }
    else 
    {
        spawnleft = true;
    }
}

static void update_traffic(double traveled, double delta_time)
{
    double traffic_top = TRAFFIC_TOP * s_height;
    double traffic_bottom = TRAFFIC_BOTTOM * s_height;
    for (int i = 0; i < s_traffic_count; i++)
    {
        double y = -s_traffic[i].speed * delta_time * DISTANCE_FACTOR;
        s_traffic[i].y += y + traveled;
        s_traffic[i].x = (double)Road_CenterOfLane(s_road, s_trafficlanes[i]);

        if (s_traffic[i].y > traffic_bottom) /* traffic is behind */
            respawn_traffic(i, traffic_top);
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
    double bestcar_moved = 0;
    if (NULL != s_bestcar)
    {
        bestcar_moved = Car_UpdateYpos(s_bestcar, delta_time, 0);
        Car_UpdateXpos(s_bestcar, delta_time, 0);
        Car_UpdateSensor(s_bestcar, s_road, s_traffic, s_traffic_count);
        s_bestcar->y += bestcar_moved;
    }



    s_updated = 0;
#ifdef LIST
    CarList_t *prev = NULL;
    CarList_t *node = s_head;
    while (NULL != node)
    {
        CarList_t *next = node->next;

        Car_UpdateXpos(node->car, delta_time, 0);
        if (node->car != s_bestcar)
        {
            Car_UpdateYpos(node->car, delta_time, -bestcar_moved);
        }

        if (is_killable(node->car))
        {
            list_remove(prev, node);
        }
        else
        {
            if (!node->car->damaged)
            {
                Car_UpdateSensor(node->car, s_road, s_traffic, s_traffic_count);
                Car_ApplyFriction(node->car, delta_time);
                Car_UpdateSpeed(node->car, delta_time);
                Car_UpdateControls(node->car);
                Car_CheckCollision(node->car, s_road, s_traffic, s_traffic_count);
            }
            prev = node;
        }

        s_updated += 1;
        node = next;
    }
#else
    for (int i = 0; i < s_car_count; i++)
    {
        Car_UpdateXpos(&s_cars[i], delta_time, 0);
        if (&s_cars[i] != s_bestcar)
        {
            Car_UpdateYpos(&s_cars[i], delta_time, bestcar_moved);
        }


        Car_UpdateSensor(&s_cars[i], s_road, s_traffic, s_traffic_count);
        if (!s_cars[i].damaged)
        {
            Car_ApplyFriction(&s_cars[i], delta_time);
            Car_UpdateSpeed(&s_cars[i], delta_time);
            Car_UpdateControls(&s_cars[i]);
            Car_CheckCollision(&s_cars[i], s_road, s_traffic, s_traffic_count);
        }
    }
#endif /* LIST */

    return bestcar_moved;
}


static double find_bestcar(void)
{
    double move_by = 0;
    
#ifdef LIST
    CarList_t *node = s_head;
    while (NULL != node)
    {
        Car_t *car = node->car;
        if (better_than_bestcar(car))
        {
            move_by = car->y - BESTCAR_YPOS();
            s_bestcar = car;
        }
        
        node = node->next;
    }
#else
    for (int i = 0; i < s_car_count; i++)
    {
        if (better_than_bestcar(&s_cars[i])) 
        {
            move_by = s_cars[i].y - BESTCAR_YPOS();
            s_bestcar = &s_cars[i];
        }
    }
#endif /* LIST */
    return move_by;
}


static void draw_cars(Color color)
{
    Color transparent = color;
    transparent.a = BYTE_PERCENTAGE(0.2);

#ifdef LIST
    CarList_t *node = s_head;
    while (NULL != node)
    {
        Car_Draw(*node->car, transparent, false, false);
        node = node->next;
    }
#else
    for (int i = 0; i < s_car_count; i++)
        Car_Draw(s_cars[i], transparent, false, false);
#endif /* LIST */

    Car_Draw(*s_bestcar, color, true, true);
    NeuralNet_Draw(s_bestcar->brain, (Rectangle){
        .x = s_road.right + 10, 
        .y = s_height*0.3, 
        .width = s_width - s_road.right - 20,
        .height = s_height*0.4f,
    });
}




static void reset(double coefficient)
{
    list_init();
    for (int i = 0; i < s_car_count; i++)
    {
        s_cars[i].damaged = false;
        s_cars[i].y = BESTCAR_YPOS();
        s_cars[i].x = Road_GetCenter(s_road);
        s_cars[i].angle = 0;
        s_cars[i].speed = 10;
        s_cars[i].traveled = 0;
        NeuralNet_Mutate(&s_cars[i].brain, s_bestcar->brain, i == 0 ? 0 : coefficient);
    }
    s_bestcar = &s_cars[0];
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
    int x = 70, y = 50;
    value_box("fps:", x, y, GetFPS());
    y += DEF_VALUEBOX_HEIGHT;
    value_box("mph:", x, y, s_bestcar->speed / 1.6);

    y += DEF_VALUEBOX_HEIGHT;
    value_box("dist:", x, y, s_bestcar->traveled * (1.0 / DISTANCE_FACTOR));

    y += DEF_VALUEBOX_HEIGHT;
    value_box("bestcar:", x, y, s_bestcar - &s_cars[0]);

    y += DEF_VALUEBOX_HEIGHT;
    value_box("updated:", x, y, s_updated);

    y += DEF_VALUEBOX_HEIGHT;
    value_box("offset:", x, y, s_bestcar->sensor.readings[0].dist * 100);
    

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
    value_box("live: ", x + 30, y, s_sp - &s_stack[0]);
    y += DEF_VALUEBOX_HEIGHT;
    value_box("total:", x + 30, y, s_car_count);


    static char tmp[256] = { 0 };
    for (int i = 0; i < s_traffic_count; i++)
    {
        y += DEF_VALUEBOX_HEIGHT;
        snprintf(tmp, sizeof tmp, "traffic %d: y:", i);
        value_box(tmp, x*2, y, s_traffic[i].y);
    }
#endif /* _DEBUG */
}



static bool is_killable(const Car_t *car)
{
    return car->speed < 0;
}

static bool better_than_bestcar(const Car_t *car)
{
    return car->y < s_bestcar->y;
}





