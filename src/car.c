

#include <raylib.h>
#include <rlgl.h>
#include <math.h>

#include "include/utils.h"
#include "include/config.h"
#include "include/car.h"
#include "include/mem.h"





static void setup_inputs(Car_t *car);






Car_t Car_Init(Car_t *car, Rectangle shape, const CarData_t data)
{
    CAI_ASSERT(car != NULL, "Car_Init does not accept NULL");
    *car = (Car_t){
        .x = shape.x,
        .y = shape.y,
        .width = shape.width, 
        .len = shape.height,
        .traveled = 0,

        .angle = 0,
        .speed = 0,
        .damaged = false,

        .direction = Control_Init(),
        .sensor = Sensor_Init(car, DEF_SENSOR_RAYCOUNT),

        .data = data,
    };

    if (data.type == CAR_AI && data.arch != NULL)
    {
        car->brain = NeuralNet_Init(*data.arch);
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
    return car->speed -= car->speed * car->data.friction * delta_time;
}


void Car_UpdateControls(Car_t *car)
{
    switch (car->data.type)
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
        if (!(car->direction.forward && car->direction.left && car->direction.right))
        {
            car->direction.reverse = bitarr_Get(output, 3);
        }
    }
    break;
    }
}



void Car_UpdateSpeed(Car_t* car, double delta_time)
{

    /* turning */
    double angle = DEF_CAR_ANGLETURN * delta_time;
    if (flt_inrange_inclusive(100, car->speed, car->data.max_speed))
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
    if (car->speed < car->data.max_speed && car->direction.forward)
    {
        double rate = car->speed >= 0 
            ? car->data.accel : car->data.decel;
        car->speed += rate * delta_time;
    }
    if (car->speed > car->data.max_reverse && car->direction.reverse)
    {
        car->speed -= car->data.decel * delta_time;
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
    Vector2 *car_points = MEM_ALLOCA_ARRAY(
        car->data.poly_count, sizeof(*car_points)
    );
    Polygon_t car_poly = Polygon_Init(car->data.poly_count, car_points);
    Car_GetPolygon(*car, &car_poly);


    /* outside of the road is considered damaged */
    if (!Polygon_InsideRect(car_poly, road.poly))
    {
        goto damaged;
    }


    if (traffic_count > 0)
    {
        Vector2 *traffic_points = MEM_ALLOCA_ARRAY(
            traffic[0].data.poly_count, sizeof(*traffic_points)
        );
        Polygon_t traffic_poly = Polygon_Init(traffic[0].data.poly_count, traffic_points);

        for (int i = 0; i < traffic_count; i++)
        {
            CAI_ASSERT(car->data.poly_count == traffic[i].data.poly_count, 
                "car and traffic should have the same poly count."
            );
            Car_GetPolygon(traffic[i], &traffic_poly);
            if (Polygon_TouchedRect(car_poly, traffic_poly))
            {
                goto damaged;
            }
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


void Car_GetPolygon(const Car_t car, Polygon_t *out_polygons)
{
    Polygon_RectFrom(out_polygons,
        (Vector2){.x = car.x, .y = car.y},
        car.width, car.len, DEG_TO_RAD(car.angle)
    );
}


void Car_GetPolygonLines(const Car_t car, Line_t *out_lines)
{
    CAI_ASSERT(car.data.poly_count == DEF_CAR_POLYCOUNT,
        "can't hangle %d polygons in GetPolygonLines",
        car.data.poly_count
    );

    Vector2 vec[4];
    Polygon_t poly = Polygon_Init(car.data.poly_count, vec);
    Car_GetPolygon(car, &poly);

    for (int i = 0; i < 4; i++)
    {
        out_lines[i].start = vec[i];
        out_lines[i].end = (i == 3) 
            ? vec[0] 
            : vec[i + 1];
    }
}







void Car_Draw(const Car_t car, Color color, bool draw_sensors, bool draw_collidebox)
{
    const flt_t x = car.x;
    const flt_t y = car.y;
    const flt_t w = car.width;
    const flt_t l = car.len;

    rlPushMatrix();
    rlTranslatef(x, y, 0);
    rlRotatef(car.angle, 0, 0, 1);
        DrawRectangle(-w/2, -l/2, w, l, color);
    rlPopMatrix();


    if (draw_collidebox)
    {
        Vector2 *lines = MEM_ALLOCA_ARRAY(car.data.poly_count, sizeof(*lines));
        Polygon_t collidebox = Polygon_Init(car.data.poly_count, lines);
        Car_GetPolygon(car, &collidebox);

        for (int i = 0; i < car.data.poly_count; i++)
        {
            Vector2 start = collidebox.points[i];
            Vector2 end = (i == car.data.poly_count - 1)
                ? collidebox.points[0]
                : collidebox.points[i + 1];
            DrawLineV(start, end, GREEN);
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
    for (usize_t i = 0; i < input->count; i++)
    {
        input->at[i] =  car->sensor.readings[i].dist;
    }
}


