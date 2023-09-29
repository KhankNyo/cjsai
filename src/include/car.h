#ifndef CAI_CAR_H
#define CAI_CAR_H


#include "control.h"
#include "common.h"
#include "network.h"
#include "sensor.h"

typedef enum ControlType_t 
{
    CAR_DUMMY,
    CAR_HUMAN,
    CAR_AI,
} ControlType_t;

struct Car_t
{
    double x, y;
    double angle, friction;
    flt_t speed, accel, decel;
    flt_t topspeed, max_reverse_spd;
    flt_t width, len;
    int poly_count;

    ControlType_t type;
    Control_t direction;
    Sensor_t sensor;
    NeuralNet_t brain;

    bool damaged;
};


/* note: shape's x and y are between 0 and 1 */
Car_t Car_Init(Car_t *car, Rectangle shape, ControlType_t type);
void Car_Deinit(Car_t *car);

void Car_Draw(const Car_t, Color color, bool draw_sensors, bool draw_collidebox);



/* manual updating functions: */



double Car_ApplyFriction(Car_t *car, double delta_time);


void Car_UpdateControls(Car_t *car);
void Car_UpdateSpeed(Car_t* car, double delta_time);

/* scales the car's position */
void Car_Reposition(Car_t* car, double xscale, double yscale);

/* returns dist traveled in the y direction */
double Car_UpdateYpos(Car_t *car, double delta_time, double y_offset);

/* returns dist traveled in the x direction */
double Car_UpdateXpos(Car_t *car, double delta_time, double x_offset);


void Car_UpdateSensor(Car_t* car, const Road_t road, const Car_t* traffic, usize_t traffic_count);
bool Car_CheckCollision(Car_t *car, const Road_t road, const Car_t *traffic, int traffic_count);

/* out_polygons must be able to contain car.poly_count elems */
void Car_GetPolygons(const Car_t car, Line_t *out_polygons);






#endif /* CAI_CAR_H */

