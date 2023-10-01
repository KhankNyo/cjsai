

#include <raylib.h>
#include <rlgl.h>
#include <math.h>

#include "include/utils.h"
#include "include/config.h"
#include "include/car.h"
#include "include/mem.h"





static void setup_inputs(Car_t *car);

Car_t Car_Init(Car_t *car, Rectangle shape, ControlType_t type)
{
    *car = (Car_t){
        .x = shape.x,
        .y = shape.y,
        .width = shape.width, 
        .len = shape.height,
        .traveled = 0,

        .angle = 0,
        .speed = 0,
        .damaged = false,

        .type = type,
        .direction = Control_Init(),
        .sensor = Sensor_Init(car, DEF_SENSOR_RAYCOUNT),

        .poly_count = DEF_CAR_POLYCOUNT,
        .friction = DEF_CAR_FRICTION,
        .topspeed = DEF_CAR_TOPSPD,
        .max_reverse_spd = -DEF_CAR_REVERSESPD,
        .accel = DEF_CAR_ACCEL,
        .decel = DEF_CAR_DECEL,
    };

    if (type == CAR_AI)
    {
        car->brain = NeuralNet_Init(DEF_NN_ARCHITECTURE, DEF_NN_SIZE);
    }
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


void Car_UpdateControls(Car_t *car)
{
    switch (car->type)
    {
    case CAR_DUMMY: return;
    case CAR_HUMAN: Control_QueryInputs(&car->direction); break;
    case CAR_AI: 
    {
        setup_inputs(car);
        bitarr_t output = NeuralNet_FeedForward(&car->brain);
        car->direction.forward = bitarr_Get(output, 0);
        car->direction.left = bitarr_Get(output, 1);
        car->direction.right = bitarr_Get(output, 2);
        car->direction.reverse = bitarr_Get(output, 3);
    }
    break;
    }
}



void Car_UpdateSpeed(Car_t* car, double delta_time)
{

    /* turning */
    double angle = DEF_CAR_ANGLETURN * delta_time;
    if (flt_inrange_inclusive(100, car->speed, car->topspeed))
        angle /= car->speed * 0.04;
    else if (flt_inrange(30, car->speed, 100))
        angle /= car->speed * 0.06;
    else if (flt_inrange_inclusive(16, car->speed, 30))
        angle /= car->speed * 0.1;
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
        double rate = car->speed >= 0 ? DEF_CAR_ACCEL : DEF_CAR_DECEL;
        car->speed += rate * delta_time;
    }
    if (car->speed > car->max_reverse_spd && car->direction.reverse)
    {
        car->speed -= DEF_CAR_DECEL * delta_time;
    }
}



void Car_Reposition(Car_t* car, double xscale, double yscale)
{
    car->x *= xscale;
    car->y *= yscale;
}





void Car_UpdateSensor(Car_t* car, const Road_t road, const Car_t* traffic, usize_t traffic_count)
{
    Sensor_Update(&car->sensor, road, traffic, traffic_count);
}


bool Car_CheckCollision(Car_t *car, const Road_t road, const Car_t *traffic, int traffic_count)
{
    Line_t* carpoly = MEM_ALLOCA_ARRAY(car->poly_count, sizeof(Line_t));
    Line_t* trafficpoly = MEM_ALLOCA_ARRAY(car->poly_count, sizeof(Line_t));
    Car_GetPolygons(*car, carpoly);

    for (int i = 0; i < car->poly_count; i++)
    {
        if ((carpoly[i].end.x < road.left || carpoly[i].start.x < road.left)
        || (carpoly[i].end.x > road.right || carpoly[i].start.x > road.right))
        {
            goto damaged;
        }

    }


    for (int i = 0; i < traffic_count; i++)
    {
        CAI_ASSERT(car->poly_count == traffic[0].poly_count, 
            "Focused car and AI should have the same poly count."
        );
        Car_GetPolygons(traffic[i], trafficpoly);
        if (Line_PolyCollide(
            carpoly, car->poly_count, 
            trafficpoly, traffic[i].poly_count))
        {
            goto damaged;
        }
    }

    return false;
damaged:
    car->speed = 0;
    return car->damaged = true;
}




double Car_UpdateXpos(Car_t *car, double delta_time, double x_offset)
{
    const double dist = car->speed * delta_time * DISTANCE_FACTOR;
    const double x = sin(DEG_TO_RAD(car->angle)) * dist;
    car->x += x + x_offset;
    return x;
}


double Car_UpdateYpos(Car_t *car, double delta_time, double y_offset)
{
    const double dist = car->speed * delta_time * DISTANCE_FACTOR;
    const double y = cos(DEG_TO_RAD(car->angle)) * dist;
    car->y -= y + y_offset;
    car->traveled += y;
    return y;
}


void Car_GetPolygons(const Car_t car, Line_t *out_polygons)
{
    CAI_ASSERT(car.poly_count == DEF_CAR_POLYCOUNT, 
        "can't handle %d polygons\n", car.poly_count
    );
    const flt_t x = car.x, 
          y = car.y, 
          w = car.width, 
          l = car.len;
    out_polygons[0] = Line_From(x - w/2,   y + l/2, w, 0); /* top */
    out_polygons[1] = Line_From(x - w/2,   y - l/2, w, 0); /* bottom */
    out_polygons[2] = Line_From(x - w/2,   y - l/2, 0, l); /* left */
    out_polygons[3] = Line_From(x + w/2,   y - l/2, 0, l); /* right */

    Vector2 center = {.x = x, .y = y};
    for (int i = 0; i < car.poly_count; i++)
    {
        out_polygons[i] = Line_Rotate(out_polygons[i], 
            center, 
            DEG_TO_RAD(car.angle)
        );
    }
}







void Car_Draw(const Car_t car, Color color, bool draw_sensors, bool draw_collidebox)
{
    const double x = car.x;
    const double y = car.y;
    const double w = car.width;
    const double l = car.len;

    rlPushMatrix();
    rlTranslatef(x, y, 0);
    rlRotatef(car.angle, 0, 0, 1);
        DrawRectangle(-w/2, -l/2, w, l, color);
    rlPopMatrix();


    if (draw_collidebox)
    {
        Line_t *lines = MEM_ALLOCA_ARRAY(car.poly_count, sizeof(*lines));
        Car_GetPolygons(car, lines);
        for (int i = 0; i < car.poly_count; i++)
        {
            DrawLineV(lines[i].start, lines[i].end, GREEN);
        }
    }
    if (draw_sensors)
    {
        Sensor_Draw(car.sensor);
    }
}








static void setup_inputs(Car_t *car)
{
    fltarr_t *input = &car->brain.levels[0].inputs;
    CAI_ASSERT(input->count == DEF_NN_ARCHITECTURE[0], 
        "%zu != %zu\n", input->count, DEF_NN_ARCHITECTURE[0]
    );
    for (usize_t i = 0; i < car->sensor.ray_count; i++)
    {
        input->at[i] = car->sensor.readings[i].dist;
    }
    input->at[input->count] = car->speed;
}
