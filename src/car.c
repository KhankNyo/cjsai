

#include <raylib.h>
#include <rlgl.h>
#include <math.h>

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
        .direction = Control_Init(),

        .friction = DEF_CAR_FRICTION,
        .topspeed = DEF_CAR_TOPSPD,
        .max_reverse_spd = -DEF_CAR_REVERSESPD,
        .accel = DEF_CAR_ACCEL,
        .decel = DEF_CAR_DECEL,
    };
    return car;
}

void Car_Deinit(Car_t *car)
{
    Control_Deinit(&car->direction);
    *car = ZEROALL(Car_t);
}





double Car_ApplyFriction(Car_t *car, double delta_time)
{
    return car->speed -= car->speed * car->friction * delta_time;
}


void Car_UpdateControls(Car_t *car, double delta_time)
{
    if (car->type == CAR_DUMMY)
        return;


    Control_QueryInputs(&car->direction);
    /* turning */
    double angle = DEF_CAR_ANGLETURN * delta_time;
    if (flt_inrange_inclusive(100, car->speed, car->topspeed))
        angle /= car->speed * 0.015;
    else if (flt_inrange(30, car->speed, 100))
        angle /= car->speed * 0.03;
    else if (flt_inrange_inclusive(16, car->speed, 30))
        angle /= car->speed * 0.06;
    else if (flt_inrange(-3, car->speed, 3))
        angle = 0.2;
    else 
        angle = 0.5;
    

    if (car->direction.left)
    {
        car->angle -= angle;
        Car_ApplyFriction(car, delta_time);
    }
    if (car->direction.right)
    {
        car->angle += angle;
        Car_ApplyFriction(car, delta_time);
    }
    if (car->speed < car->topspeed && car->direction.forward)
    {
        double rate = car->speed > 0 ? car->accel : car->decel;
        car->speed += rate * delta_time;
    }
    if (car->speed > car->max_reverse_spd && car->direction.reverse)
    {
        car->speed -= car->decel * delta_time;
    }
}




double Car_UpdateXpos(Car_t *car, int screen_w, double delta_time)
{
    const double dist = car->speed * delta_time * DISTANCE_FACTOR;
    const double x = sin(DEG_TO_RAD(car->angle)) * dist;
    car->relx += x / screen_w;
    return x;
}


double Car_UpdateYpos(Car_t *car, int screen_h, double delta_time)
{
    const double dist = car->speed * delta_time * DISTANCE_FACTOR;
    const double y = cos(DEG_TO_RAD(car->angle)) * dist;
    car->rely += y / screen_h;
    return y;
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

