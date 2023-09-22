

#include <raylib.h>
#include <raygui.h>

#include "include/config.h"
#include "include/common.h"
#include "include/cai.h"
#include "include/road.h"


static Font s_font = { 0 };
static const float s_font_space = 1;

static Road_t s_road = { 0 };

static void cai_draw_text(const char *text, int x, int y);


void CAI_Init(void)
{
    InitWindow(DEF_WIN_WIDTH, DEF_WIN_HEIGHT, "Hello, world");
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
}


bool CAI_IsRunning(void)
{
    return !WindowShouldClose();
}


void CAI_Run(void)
{
    static float i = 0; 
    static float velocity = 100;
    int win_h = GetRenderHeight();
    int win_w = GetRenderWidth();

    /* logic code */
    Road_Recenter(&s_road, win_w / 2);
    GuiSlider((Rectangle)
        {.x = 100, .y = (float)win_h / 2, .width = 100, .height = 10}, 
        "min (100)", "max (1000)", &velocity, 100, 1000
    );
    i += velocity * GetFrameTime();
    if ((int)i >= s_road.dash_len*2)
        i -= s_road.dash_len*2;


    /* rendering code */
    BeginDrawing();
        ClearBackground(RAYWHITE);
        Road_Draw(s_road, 0, win_h, i);
        DrawFPS(0, 0);
    EndDrawing();
}


void CAI_Deinit(void)
{
    Road_Deinit(&s_road);
    UnloadFont(s_font);
    CloseWindow();
}






static void cai_draw_text(const char *text, int x, int y)
{
    DrawTextEx(s_font, text, 
        (Vector2){.x = x, .y = y},
        s_font.baseSize, 
        s_font_space, BLACK
    );
}

