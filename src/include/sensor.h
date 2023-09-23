#ifndef CAI_SENSOR_H
#define CAI_SENSOR_H



#include "common.h"
#include "utils.h"


typedef struct Sensor_t
{
    const Car_t *car;
    int ray_len;
    flt_t ray_spread;
    int ray_count;

    Coord_t *rays;
} Sensor_t;



Sensor_t Sensor_Init(const Car_t *car, int ray_count);
void Sensor_Deinit(Sensor_t *sensor);




#endif /* CAI_SENSOR_H */

