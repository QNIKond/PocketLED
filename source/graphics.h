#ifndef GRAPHICS_H_
#define GRAPHICS_H_
#include <stdint.h>
#include <avr/pgmspace.h>

extern const uint8_t _NeoPixelGammaTable[256];
#define GAMMA(X) pgm_read_byte(&_NeoPixelGammaTable[X])

void gradientScreen();
void gradientWGammaScreen();

void drawLetter(uint8_t (*sc)[16], int8_t x, int8_t y, uint8_t letter);

#define TEXTSCROLLSPEED 16
#define TEXTINNIT 40
void drawRunningText(uint8_t (*sc)[16], uint8_t t, int8_t y, const char* str, uint8_t len);

void drawRunningTitle(uint8_t (*sc)[16], uint8_t t, int8_t y, const char* str, uint8_t len);

#endif /* GRAPHICS_H_ */