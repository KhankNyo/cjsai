#include "include/cai.h"

int main(void)
{
    CAI_Init();
    while (CAI_IsRunning())
    {
        CAI_Run();
    }
    CAI_Deinit();
    return 0;
}




