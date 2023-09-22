

#include <raylib.h>

#include "include/config.h"
#include "include/common.h"
#include "include/cai.h"


static Font s_font = { 0 };
static const float s_font_space = 1;

static void cai_draw_text(const char *text, int x, int y);


void CAI_Init(void)
{
    InitWindow(1080, 720, "Hello, world");
    SetTargetFPS(60);
    s_font = LoadFont(CAI_FONT_FILE);
}


bool CAI_IsRunning(void)
{
    return !WindowShouldClose();
}


void CAI_Run(void)
{
    /* logic code */


    /* rendering code */
    BeginDrawing();
        ClearBackground(RAYWHITE);
        cai_draw_text("Hello, world", 0, 0);
    EndDrawing();
}


void CAI_Deinit(void)
{
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

