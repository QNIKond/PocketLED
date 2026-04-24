#ifndef LEDMATRIX_H_
#define LEDMATRIX_H_
#include <stdint.h>
#include <avr/pgmspace.h>

extern const uint8_t _NeoPixelGammaTable[256] ;
#define GAMMA(X) pgm_read_byte(&_NeoPixelGammaTable[X])

extern uint8_t (*canvas)[16];

extern uint64_t mtime;

void LEDMatrixSetup();

void flushScreenAndWait();

#endif /* LEDMATRIX_H_ */