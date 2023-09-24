#ifndef CAI_SENSOR_H
#define CAI_SENSOR_H



#include <raylib.h>

#include "common.h"
#include "utils.h"
#include "road.h"





typedef struct Sensor_t
{
    const Car_t *car;
    double ray_spread, ray_len;

    int ray_count;
    Line_t *rays;
    Reading_t *readings;

    struct {
        Color normal, touched;
    } color;
} Sensor_t;



Sensor_t Sensor_Init(const Car_t *car, int ray_count);
void Sensor_Update(
    Sensor_t *sensor, const Road_t road, const Car_t *cars, int car_count
);
void Sensor_Draw(const Sensor_t sensor, int win_w);
void Sensor_Deinit(Sensor_t *sensor);




#endif /* CAI_SENSOR_H */

