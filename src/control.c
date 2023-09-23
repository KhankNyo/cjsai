

#include <raylib.h>
#include "include/control.h"



void Control_QueryInputs(Control_t *control)
{
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
        control->forward = true;
    else if (IsKeyUp(KEY_W) || IsKeyUp(KEY_UP))
        control->forward = false;

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
        control->left = true;
    else if (IsKeyUp(KEY_A) || IsKeyUp(KEY_LEFT))
        control->left = false;

    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
        control->right = true;
    else if (IsKeyUp(KEY_D) || IsKeyUp(KEY_RIGHT))
        control->right = false;

    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
        control->reverse = true;
    else if (IsKeyUp(KEY_S) || IsKeyUp(KEY_DOWN))
        control->reverse = false;
}


