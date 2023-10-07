#ifndef CAI_SAVE_H
#define CAI_SAVE_H



#include "network.h"

/* returns true if file already exist, 
 * if file does not exist, a new file will be created */
bool Saver_SaveFile(const char *filename, const NeuralNet_t nn);
NeuralNet_t Saver_LoadSave(const char *filename);



#endif /* CAI_SAVE_H */

