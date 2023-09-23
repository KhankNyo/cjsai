#ifndef CAI_CAR_H
#define CAI_CAR_H


#include <raylib.h>

#include "control.h"
#include "common.h"

typedef enum ControlType_t 
{
    CAR_DUMMY,
    CAR_HUMAN,
    CAR_AI,
} ControlType_t;
typedef struct Car_t
{
    double relx, rely;
    double angle, friction;
    float speed, accel, decel;
    float topspeed, max_reverse_spd;
    float width, len;

    ControlType_t type;
    Control_t direction;

    Color color;
    bool damaged;
} Car_t;


/* note: shape's x and y are between 0 and 1 */
Car_t Car_Init(Rectangle shape, Color color, ControlType_t type);
void Car_Deinit(Car_t *car);


double Car_ApplyFriction(Car_t *car, double delta_time);
void Car_UpdateControls(Car_t *car, double delta_time);

/* returns dist traveled in the y direction */
double Car_UpdateYpos(Car_t *car, int screen_height, double delta_time);

/* returns dist traveled in the x direction */
double Car_UpdateXpos(Car_t *car, int screen_width, double delta_time);

void Car_Draw(const Car_t, int win_w, int win_h);


#endif /* CAI_CAR_H */

