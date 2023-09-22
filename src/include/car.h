#ifndef CAI_CAR_H
#define CAI_CAR_H


#include <raylib.h>
#include "common.h"

typedef enum ControlType_t 
{
    CAR_DUMMY,
    CAR_HUMAN,
    CAR_AI,
} ControlType_t;
typedef struct Car_t
{
    float speed;
    double relx, rely;
    float width, len;
    double angle;
    ControlType_t type;
    Color color;
    bool damaged;
} Car_t;


/* note: shape's x and y are between 0 and 1 */
Car_t Car_Init(Rectangle shape, Color color, ControlType_t type);
void Car_Deinit(Car_t *car);
void Car_Draw(const Car_t, int win_w, int win_h);


#endif /* CAI_CAR_H */

