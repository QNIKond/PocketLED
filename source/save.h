#ifndef SAVE_H_
#define SAVE_H_
#include "os.h"
#include <stdint.h>

#define SAVEDATA 2
#define SAVEVALIDATION 165

void saveSetup();

void saveGame(Game *game);

uint8_t restoreGame(Game *game, uint8_t* mem);

void eraseSave(Game *game);

#endif /* SAVE_H_ */