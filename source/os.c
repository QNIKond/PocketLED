#include "os.h"
#include "common.h"
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "LEDMatrix.h"
#include "input.h"
#include "UARTDebug.h"
#include "sound.h"

static volatile uint8_t dt;

#define X(A) &A,
Game *games[] = {XGAMES};
#undef X

void osSetup(){
	cli();
	inputSetup();
	LEDMatrixSetup(&dt);
	UARTSetup();
	soundSetup();
	sei();
}

void gradient(){
	for(uint8_t i = 0; i < 16; ++i)
	for(uint8_t j = 0; j < 16; ++j)
	canvas[i][j] = (i<<4) + j;
}


void gradientWGamma(){
	for(uint8_t i = 0; i < 16; ++i)
	for(uint8_t j = 0; j < 16; ++j)
	canvas[i][j] = GAMMA((i<<4) + j);
}

void osRun(){
	while (1){
		if (inputUp&INPUP){
			playNote(&tnote1, 128);
		}
		if (inputUp&INPLEFT){
			playNote(&tnote2, 192);
		}
		
		gradientWGamma();
		
		flushScreenAndWait();
		updateInput(dt);
		dt = 0;
	}
}