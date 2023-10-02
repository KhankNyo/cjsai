

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
#include "include/save.h"



static Font s_font = { 0 };
static int s_font_size = DEF_FONT_SIZE;

static Road_t s_road = { 0 };

static Car_t s_traffic[DEF_TRAFFIC_COUNT] = { 0 };
static Color s_traffic_color;
static int s_trafficlanes[DEF_TRAFFIC_COUNT] = { 0 };
static int s_traffic_count = DEF_TRAFFIC_COUNT;


static NNArch_t s_arch = {0};
static Car_t s_cars[15000] = { 0 };
static Color s_car_color;
static int s_car_count = STATIC_ARRSIZE(s_cars);


static Car_t *s_bestcar = NULL;
static NeuralNet_t *s_bestbrain = NULL;
static int s_kill_threshold = DEF_KILL_THRESHOLD;



static int s_height = DEF_WIN_HEIGHT;
#define BESTCAR_YPOS() (s_height*0.7)
#define GET_KILL_THRESHOLD() (s_height * s_kill_threshold)
static int s_width = DEF_WIN_WIDTH;




typedef struct CarList_t
{
    Car_t *car;
    struct CarList_t *next;
} CarList_t;
static CarList_t *s_pool = NULL;
static CarList_t **s_stack = NULL;
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
static void save_bestcar(void);

static int random_lane_index(void);
static double random_lane(int *outlane);


static Rectangle defbox(flt_t x, flt_t y);
static Rectangle box(flt_t x, flt_t y, flt_t w, flt_t h);
static void draw_info(void);
static void value_box(const char *title, int x, int y, int value);

static bool is_killable(const Car_t *car);
static bool better_than_bestcar(const Car_t *car);





void CAI_Init(void)
{
    s_bestbrain = &s_cars[0].brain;
    s_arch = NNArch_Init(DEF_NN_ARCH_SIZE, DEF_NN_ARCH_LEVELS);

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
    GuiSetStyle(DEFAULT, TEXT_SIZE, s_font_size);


    s_road = Road_Init(DEF_WIN_WIDTH / 2, 
        DEF_ROAD_WIDTH, 
        DEF_WIN_HEIGHT,
        DEF_ROAD_LANES
    );


    flt_t rely = TRAFFIC_TOP;
    int traffic_per_lane = s_road.numlanes - 1;
    memcpy(s_trafficlanes, 
        (int[]){2, 1, 1, 0, 1, 2, 1, 0, 2, 1}, 
        sizeof s_trafficlanes
    );
    for (int i = 0, occupied = 0; i < s_traffic_count; i++, occupied++)
    {
        if (occupied == traffic_per_lane)
        {
            rely -= TRAFFIC_TOP * traffic_per_lane/s_traffic_count;
            occupied = 0;
        }
        s_traffic[i] = default_traffic(&s_traffic[i], 
            s_trafficlanes[i], 
            rely * s_height
        );
    }

    for (int i = 0; i < s_car_count; i++)
    {
        s_cars[i] = Car_Init(
            &s_cars[i],
            DEF_CAR_RECT(
                DEF_WIN_WIDTH*0.5, BESTCAR_YPOS()), 
            CarData_Default(CAR_AI, &s_arch)
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

    flt_t x = s_width * .8;
    GuiSlider(box(x, s_height*.75, 5, DEF_VALUEBOX_HEIGHT), 
        "similar", "different", &coefficient, 0, 1
    );
    value_box("", x, s_height*0.78, coefficient * 100);
    if (GuiButton(
        box(x, s_height*.81, (flt_t)sizeof("Mutate") / 2, DEF_VALUEBOX_HEIGHT), 
        "Mutate"))
    {
        reset(coefficient);
    }
    if (GuiButton(defbox(x, s_height*.84), "Save"))
    {
        save_bestcar();
    }


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
    NNArch_Deinit(&s_arch);
    NeuralNet_Deinit(s_bestbrain);
    Road_Deinit(&s_road);
    UnloadFont(s_font);
    CloseWindow();
}









static void pool_init(int elems)
{
    s_pool = MEM_ALLOC_ARRAY(elems, sizeof(s_pool[0]));
    s_stack = MEM_ALLOC_ARRAY(elems, sizeof(s_stack[0]));
    s_poolsize = elems;
}


static void pool_deinit(void)
{
    mem_free(s_pool);
    mem_free(s_stack);

    s_pool = NULL;
    s_poolsize = 0;
    s_stack = NULL;
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





static Car_t default_traffic(Car_t *traffic, int lane, int y)
{
    *traffic = Car_Init(
        traffic,
        DEF_CAR_RECT(
            Road_CenterOfLane(s_road, lane), 
            y
        ), 
        CarData_Default(CAR_DUMMY, NULL)
    );
    traffic->speed = DEF_TRAFFIC_SPD;
    return *traffic;
}


static void respawn_traffic(int i, double spawn)
{
    s_traffic[i].y = spawn;
    s_traffic[i].x = Road_CenterOfLane(s_road, s_trafficlanes[i]);
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
    bestcar_moved = Car_UpdateYpos(s_bestcar, delta_time, 0);
    Car_UpdateXpos(s_bestcar, delta_time, 0);
    Car_UpdateSensor(s_bestcar, s_road, s_traffic, s_traffic_count);
    s_bestcar->y = BESTCAR_YPOS();



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

        node = next;
    }
    return bestcar_moved;
}


static double find_bestcar(void)
{
    double move_by = 0;
    
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
    return move_by;
}


static void draw_cars(Color color)
{
    Color transparent = color;
    transparent.a = BYTE_PERCENTAGE(0.2);

    CarList_t *node = s_head;
    while (NULL != node)
    {
        Car_Draw(*node->car, transparent, false, false);
        node = node->next;
    }

    Car_Draw(*s_bestcar, color, true, true);
    Color background = (Color){.r=37, .g=26, .b=9, .a=BYTE_PERCENTAGE(1)};
    NeuralNet_Draw(
        s_bestcar->brain, 
        (Rectangle){
            .x = s_road.right + 10, 
            .y = s_height*0.3, 
            .width = s_width - s_road.right - 20,
            .height = s_height*0.4f,
        },
        background,
        DEF_LEVEL_NODE_RADIUS
    );
    CAI_ASSERT(NULL != s_bestbrain, "draw cars");

    flt_t w = (flt_t)(s_width - s_road.right) / 2;
    NeuralNet_Draw(
        *s_bestbrain,
        (Rectangle){
            .x = s_road.right + w/2,
            .y = s_height*0.05,
            .width = w,
            .height = s_height*0.2,
        },
        background,
        DEF_LEVEL_NODE_RADIUS / 2
    );
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

        CAI_ASSERT(s_bestbrain != NULL, "bestbrain must not be null on reset");
        if (&s_cars[i].brain == s_bestbrain)
            s_bestcar = &s_cars[i];
        else
            NeuralNet_Mutate(&s_cars[i].brain, *s_bestbrain, i == 0 ? 0 : coefficient);
    }
}



static void save_bestcar(void)
{
    s_bestbrain = &s_bestcar->brain;
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




static Rectangle box(flt_t x, flt_t y, flt_t w, flt_t h)
{
    return (Rectangle){
        .x = x, 
        .y = y,
        .width = w * s_font_size,
        .height = h * s_font_size,
    };
}

static Rectangle defbox(flt_t x, flt_t y)
{
    return box(x, y, DEF_VALUEBOX_WIDTH, DEF_VALUEBOX_HEIGHT);
}


static void value_box(const char *title, int x, int y, int val)
{
    GuiValueBox(
        box(x, y, DEF_VALUEBOX_WIDTH, DEF_VALUEBOX_HEIGHT),
        title, &val, 0, val, false
    );
}


static void draw_info(void)
{
#define NEXT_BOX() (y += (DEF_VALUEBOX_HEIGHT * s_font_size))
    int x = s_font_size * 5, y = s_font_size * 3;
    value_box("fps:", x, y, GetFPS());
    NEXT_BOX();
    value_box("mph:", x, y, s_bestcar->speed / 1.6);

    NEXT_BOX();
    value_box("dist:", x, y, s_bestcar->traveled * (1.0 / DISTANCE_FACTOR));

    NEXT_BOX();
    value_box("bestcar:", x, y, s_bestcar - &s_cars[0]);

    NEXT_BOX();
    value_box("offset:", x, y, s_bestcar->sensor.readings[0].dist * 100);

    NEXT_BOX();
    value_box("live: ", x, y, s_sp - &s_stack[0]);
    NEXT_BOX();
    value_box("total:", x, y, s_car_count);
    

#ifdef _DEBUG

    NEXT_BOX();
    value_box("angle:", x, y, s_bestcar->angle);

    NEXT_BOX();
    value_box("xpos:", x, y, s_bestcar->x);

    NEXT_BOX();
    value_box("ypos:", x, y, s_bestcar->y);

    NEXT_BOX();
    value_box("dmg: ", x, y, s_bestcar->damaged);



    static char tmp[256] = { 0 };
    for (int i = 0; i < s_traffic_count; i++)
    {
        NEXT_BOX();
        snprintf(tmp, sizeof tmp, "traffic %d: y:", i);
        value_box(tmp, x*2, y, s_traffic[i].y);
    }
#endif /* _DEBUG */
}







static bool is_killable(const Car_t *car)
{
    return car->speed < 0 || car->damaged || car->y > GET_KILL_THRESHOLD();
}

static bool better_than_bestcar(const Car_t *car)
{
    return car->y < s_bestcar->y && car->traveled > s_bestcar->traveled;
}





