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
#include "graphics.h"

static volatile uint8_t dt;

#define X(A) &A,
Game *games[] = {XGAMES};
#undef X
#define X(A) "A",
const char* gameNames[] = {XGAMES};
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
	uint8_t x = 0;
	int8_t y = 10;
	while (1){
		if (inputUp&INPUP){
			--y;
		}
		if (inputUp&INPLEFT){
			--x;
		}
		if (inputUp&INPDOWN){
			++y;
		}
		if (inputUp&INPRIGHT){
			++x;
		}
		x %= 6*4;
		drawRunningText(canvas, x%(6*4),0,"Tetris",6);
		drawRunningText(canvas, x%(4*4),6,"2048",4);
		drawRunningText(canvas, x%(2*4),12,"Q ",2);
		
		flushScreenAndWait();
		updateInput(dt);
		dt = 0;
	}
}