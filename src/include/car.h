#ifndef CAI_CAR_H
#define CAI_CAR_H


#include "control.h"
#include "common.h"
#include "network.h"
#include "sensor.h"
#include "line.h"

typedef enum ControlType_t 
{
    CAR_DUMMY,
    CAR_HUMAN,
    CAR_AI,
} ControlType_t;


/* things that the car does not modify */
typedef struct CarData_t
{
    ControlType_t type;
    NNArch_t *arch; /* not an owning ptr */

    flt_t friction;
    flt_t max_reverse, max_speed;
    flt_t accel, decel;

    int poly_count;
} CarData_t;


#define CarData_Default(ctrltype, weakptr_arch) (CarData_t){\
    .type = ctrltype,\
    .arch = weakptr_arch,\
    .friction = DEF_CAR_FRICTION,\
    .max_reverse = -DEF_CAR_MAX_REVERSE,\
    .max_speed = DEF_CAR_MAX_SPEED,\
    .accel = DEF_CAR_ACCEL,\
    .decel = DEF_CAR_DECEL,\
    .poly_count = DEF_CAR_POLYCOUNT,\
}


struct Car_t
{
    flt_t x, y;
    flt_t traveled;

    flt_t angle;
    flt_t speed;
    flt_t width, len;

    Control_t direction;
    Sensor_t sensor;
    NeuralNet_t brain;
    CarData_t data;

    bool damaged;
};


/* note: shape's x and y are between 0 and 1 */
Car_t Car_Init(Car_t *car, Rectangle shape, const CarData_t data);
void Car_Deinit(Car_t *car);

void Car_Draw(const Car_t, Color color, bool draw_sensors, bool draw_collidebox);



/* manual updating functions: */

flt_t Car_ApplyFriction(Car_t *car, flt_t delta_time);


void Car_UpdateControls(Car_t *car);
void Car_UpdateSpeed(Car_t* car, flt_t delta_time);

/* scales the car's position */
void Car_Reposition(Car_t* car, flt_t xscale, flt_t yscale);

/* returns dist traveled in the y direction */
flt_t Car_UpdateYpos(Car_t *car, flt_t delta_time, flt_t y_offset);

/* returns dist traveled in the x direction */
flt_t Car_UpdateXpos(Car_t *car, flt_t delta_time, flt_t x_offset);


void Car_UpdateSensor(Car_t* car, const Road_t road, const Car_t* traffic, usize_t traffic_count);
bool Car_CheckCollision(Car_t *car, const Road_t road, const Car_t *traffic, int traffic_count);

/* out_polygons->points must be able to contain car.poly_count vectors */
void Car_GetPolygon(const Car_t car, Polygon_t *out_polygons);

/* out_lines must be able to contain car.poly_count lines */
void Car_GetPolygonLines(const Car_t car, Line_t *out_lines);






#endif /* CAI_CAR_H */

