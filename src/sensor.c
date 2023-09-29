

#include <math.h>
#include <string.h> /* memset */
#include <raylib.h>

#include "include/config.h"
#include "include/sensor.h"
#include "include/mem.h"
#include "include/car.h"




/* updates the ray positions */
static void cast_rays(Sensor_t *sensor);
static Reading_t get_shortest_reading(
    const Car_t *current, Line_t ray, Road_t road, const Car_t *cars, usize_t car_count
);


Sensor_t Sensor_Init(const Car_t *car, usize_t ray_count)
{
    Sensor_t sensor = {
        .car = car,
        .ray_count = ray_count,
        .rays = NULL,

        .ray_len = DEF_SENSOR_RAYLEN,
        .ray_spread = DEF_SENSOR_RAYSPREAD,
        .color.normal = DEF_SENSOR_COLORNORMAL,
        .color.touched = DEF_SENSOR_COLORTOUCHED,
    };
    sensor.rays = MEM_ALLOC_ARRAY(ray_count, sizeof(*sensor.rays));
    sensor.readings = MEM_ALLOC_ARRAY(ray_count, sizeof(*sensor.readings));
    return sensor;
}





void Sensor_Update(
    Sensor_t *sensor, const Road_t road, 
    const Car_t *cars, usize_t car_count
)
{
    cast_rays(sensor);
    for (usize_t i = 0; i < sensor->ray_count; i++)
    {
        sensor->readings[i] = get_shortest_reading(
            sensor->car,
            sensor->rays[i], 
            road,
            cars, car_count
        );
    }
}


void Sensor_Draw(const Sensor_t sensor)
{
    for (usize_t i = 0; i < sensor.ray_count; i++)
    {
        Line_t ray = sensor.rays[i];

        if (0 == sensor.readings[i].dist)
        {
            DrawLineV(ray.start, ray.end, sensor.color.normal);
        }
        else
        {
            Vector2 touched = {
                .x = sensor.readings[i].at.x, 
                .y = sensor.readings[i].at.y,
            };
            DrawLineV(ray.start, touched, sensor.color.normal);
            DrawLineV(touched, ray.end, sensor.color.touched);
        }
    }
}




void Sensor_Deinit(Sensor_t *sensor)
{
    MEM_FREE_ARRAY(sensor->rays);
    MEM_FREE_ARRAY(sensor->readings);
    *sensor = ZEROALL(Sensor_t);
}








static void cast_rays(Sensor_t *sensor)
{
    const Car_t *car = sensor->car;
    for (usize_t i = 0; i < sensor->ray_count; i++)
    {
        double ray_position = sensor->ray_count == 1 
            ? 0.5f 
            : (flt_t)i / (sensor->ray_count - 1);
        double ray_angle = car->angle + LERP(
            sensor->ray_spread/2,
            -sensor->ray_spread/2,
            ray_position
        );
        ray_angle = -DEG_TO_RAD(ray_angle);

        sensor->rays[i] = (Line_t){
            .start.x = car->x,
            .start.y = car->y,
            .end.x = (double)car->x - sin(ray_angle) * sensor->ray_len,
            .end.y = (double)car->y - cos(ray_angle) * sensor->ray_len,
        };
    }
}



static Reading_t get_shortest_reading(
    const Car_t *current, 
    Line_t ray, Road_t road, 
    const Car_t *cars, usize_t car_count)
{
    Reading_t last = { 0 };
    Reading_t touch = { 0 };
    /* distance from the ray's origin to the touched object is never zero, 
     * because the ray is casted from the car's center 
     * so if it is zero, we're not touching anything */

    /* check for usize_tersection with road border */
    {
        Line_Intersect(&touch, ray, Road_RightBorder(road));

        last = touch;
        if (Line_Intersect(&touch, ray, Road_LeftBorder(road)) 
        && 0 != last.dist 
        && last.dist < touch.dist)
        {
            touch = last;
        }
    }
    



    /* check for usize_tersection with cars */
    for (usize_t i = 0; i < car_count; i++)
    {
        if (&cars[i] == current)
            continue;

        Line_t *polygons = MEM_ALLOCA_ARRAY(cars[i].poly_count, sizeof(*polygons));
        Car_GetPolygons(cars[i], polygons);
        /* iterate through the car's polygons */
        for (int j = 0; j < cars[i].poly_count; j++) /* fuck i and j */
        {
            last = touch;
            if (Line_Intersect(&touch, ray, polygons[j]) 
            && 0 != last.dist 
            && last.dist < touch.dist)
            {
                touch = last;
            }
        }
    }
    return touch;
}




