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
/* returns true if able to load save file, false otherwise */
bool Saver_LoadSave(NeuralNet_t *nn, const char *filename);



#endif /* CAI_SAVE_H */

