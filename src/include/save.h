#ifndef CAI_SAVE_H
#define CAI_SAVE_H



#include "network.h"


typedef enum SaverStatus_t
{
    SAVE_FAILED = 0,
    SAVE_NEW_FILE,
    SAVE_OLD_FILE,
} SaverStatus_t;

SaverStatus_t Saver_SaveFile(const char *filename, const NeuralNet_t nn);
NeuralNet_t Saver_LoadSave(const char *filename);



#endif /* CAI_SAVE_H */

