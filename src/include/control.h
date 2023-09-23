#ifndef CAI_CONTROL_H
#define CAI_CONTROL_H



#include "common.h"
typedef struct Control_t
{
    bool forward, reverse, left, right;
} Control_t;


#define Control_Init() (Control_t){0}
#define Control_Deinit(p_control) \
    do { (*p_control) = Control_Init(); } while (0)

void Control_QueryInputs(Control_t *control);

#endif /* CAI_CONTROL_H */

