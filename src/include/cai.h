#ifndef CAI_H
#define CAI_H


#include "common.h"

/* initializes states and libraries */
void CAI_Init(void);

/* query user input and check if the window is not closed */
bool CAI_IsRunning(void);

/* run logic and render graphics */
void CAI_Run(void);

/* deinitializes states and libraries */
void CAI_Deinit(void);


#endif /* CAI_H */

