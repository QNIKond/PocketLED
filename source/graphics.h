#ifndef GRAPHICS_H_
#define GRAPHICS_H_
#include <stdint.h>
#include <avr/pgmspace.h>

extern const uint8_t _NeoPixelGammaTable[256];
#define GAMMA(X) (pgm_read_byte(&_NeoPixelGammaTable[X]))

#define HLINE(C, Y, X1, X2, B) for (uint8_t ihline = X1; ihline<=X2; ++ihline){C[Y][ihline] = B;}

extern volatile uint8_t (*canvas)[16];

void gradientScreen();
void gradientWGammaScreen();

void drawLetter(int8_t x, int8_t y, uint8_t letter);

#define TEXTSCROLLSPEED 13
#define TEXTINNIT 50
void drawRunningText(uint8_t t, int8_t y, const char* str, uint8_t len);

void drawRunningTitle(uint8_t t, int8_t y, const char* str, uint8_t len);

#endif /* GRAPHICS_H_ */