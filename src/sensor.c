

#include "include/config.h"
#include "include/sensor.h"
#include "include/mem.h"




Sensor_t Sensor_Init(const Car_t *car, int ray_count)
{
    Sensor_t sensor = {
        .car = car,
        .ray_count = ray_count,
        .rays = NULL,

        .ray_len = DEF_SENSOR_RAYLEN,
        .ray_spread = DEF_SENSOR_RAYSPREAD,
    };
    sensor.rays = MEM_ALLOC_ARRAY(ray_count, sizeof(*sensor.rays));
    return sensor;
}


void Sensor_Deinit(Sensor_t *sensor)
{
    MEM_FREE_ARRAY(sensor->rays);
    *sensor = ZEROALL(Sensor_t);
}

