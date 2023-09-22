

#include <raylib.h>
#include <rlgl.h>

#include "include/utils.h"
#include "include/config.h"
#include "include/car.h"


Car_t Car_Init(Rectangle shape, Color color, ControlType_t type)
{
    Car_t car = {
        .relx = shape.x,
        .rely = shape.y,
        .width = shape.width, 
        .len = shape.height,
        .angle = 0,
        .speed = 0,
        .damaged = false,
        .type = type,
        .color = color,
    };
    return car;
}

void Car_Deinit(Car_t *car)
{
    *car = ZEROALL(Car_t);
}

void Car_Draw(const Car_t car, int win_w, int win_h)
{
    const double x = win_w * car.relx;
    const double y = win_h * car.rely;

    rlPushMatrix();
    rlTranslatef(x, y, 0);
    rlRotatef(car.angle, 0, 0, 1);
        DrawRectangle(-car.width/2, -car.len/2, car.width, car.len, car.color);
    rlPopMatrix();
}

