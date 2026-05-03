#ifndef GRAPHICS_H_
#define GRAPHICS_H_
#include <stdint.h>

void drawLetter(uint8_t (*sc)[16], int8_t x, int8_t y, uint8_t letter);

void drawRunningText(uint8_t (*sc)[16], uint8_t t, int8_t y, const char* str, uint8_t len);

#endif /* GRAPHICS_H_ */