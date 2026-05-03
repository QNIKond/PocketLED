#ifndef LEDMATRIX_H_
#define LEDMATRIX_H_
#include <stdint.h>
#include <avr/pgmspace.h>

extern uint8_t (*canvas)[16];

void LEDMatrixSetup(volatile uint8_t* frameCount);

void flushScreenAndWait();

#endif /* LEDMATRIX_H_ */