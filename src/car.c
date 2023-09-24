

#include <raylib.h>
#include <rlgl.h>
#include <math.h>

#include "include/utils.h"
#include "include/config.h"
#include "include/car.h"


Car_t Car_Init(Car_t *car, Rectangle shape, Color color, ControlType_t type)
{
    *car = (Car_t){
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
        .sensor = Sensor_Init(car, DEF_SENSOR_RAYCOUNT),

        .poly_count = DEF_CAR_POLYCOUNT,
        .friction = DEF_CAR_FRICTION,
        .topspeed = DEF_CAR_TOPSPD,
        .max_reverse_spd = -DEF_CAR_REVERSESPD,
        .accel = DEF_CAR_ACCEL,
        .decel = DEF_CAR_DECEL,
    };
    return *car;
}

void Car_Deinit(Car_t *car)
{
    Control_Deinit(&car->direction);
    Sensor_Deinit(&car->sensor);
    *car = ZEROALL(Car_t);
}





double Car_ApplyFriction(Car_t *car, double delta_time)
{
    return car->speed -= car->speed * car->friction * delta_time;
}


void Car_UpdateControls(Car_t *car, double delta_time)
{
    switch (car->type)
    {
    case CAR_DUMMY: return;
    case CAR_HUMAN: Control_QueryInputs(&car->direction); break;
    case CAR_AI: break;
    }

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



void Car_UpdateSensor(Car_t *car, const Road_t road, const Car_t *traffic, int traffic_count)
{
    Sensor_Update(&car->sensor, road, traffic, traffic_count);
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


void Car_GetPolygons(const Car_t car, Line_t *out_polygons)
{
    CAI_ASSERT(car.poly_count == DEF_CAR_POLYCOUNT, 
        "can't handle %d polygons\n", car.poly_count
    );
    const flt_t x = car.relx, 
          y = car.rely, 
          w = car.width, 
          l = car.len;
    out_polygons[0] = Line_From(x,        y,      x,      y + l);
    out_polygons[1] = Line_From(x + w,    y,      x + w,  y + l);
    out_polygons[2] = Line_From(x,        y,      x + w,  y);
    out_polygons[3] = Line_From(x,        y + l,  x,      y + l);
}




void Car_Draw(const Car_t car, int scale, bool draw_sensors)
{
    const double x = scale * car.relx;
    const double y = scale * car.rely;
    const double w = scale * car.width;
    const double l = scale * car.len;

    rlPushMatrix();
    rlTranslatef(x, y, 0);
    rlRotatef(car.angle, 0, 0, 1);
        DrawRectangle(-w/2, -l/2, w, l, car.color);
    rlPopMatrix();

    if (draw_sensors)
        Sensor_Draw(car.sensor, scale);
}

