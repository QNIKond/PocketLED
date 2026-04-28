/*
 * MatrixTest3.c
 *
 * Created: 04.03.2026 9:06:59
 * Author : NIKond
 */ 

#include "common.h"
#include <avr/io.h>
#include <stdint.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <string.h>
#include "LEDMatrix.h"
#include "input.h"
#include "UARTDebug.h"
#include "sound.h"

#define FRQ(X) ((F_CPU / (2UL * 8 * X)) - 1)

void setup(){
	cli();
	inputSetup();
	LEDMatrixSetup();
	UARTSetup();
	soundSetup();
	sei();
}

void onePixelTest(){
	static uint8_t c = 0;
	static uint8_t r = 0;
	canvas[r][c] = 255;
	c = (c+1);
	if(c >= 16)
	{
		c = 0;
		r = (r+1)%16;
	}
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

const Note tnote1 PROGMEM = {
	FREQTOSTEP(1000),
	DECAYCURVE(1000, 255),
	.grain = 128,
};

const Note tnote2 PROGMEM = {
	FREQTOSTEP(1000),
	DECAYCURVE(1000, 255),
	.grain = 255,
};

#define MSPT 20
int main(void){
	setup();
	
	uint64_t prevTime = mtime;
	//playNote(&tnote, 128);
    while (1){
		if(inputUp&INPUP){
			playNote(&tnote1, 128);
			
		}
		if (inputUp&INPLEFT){
			playNote(&tnote2, 198);
		}
		
		if(inputRaw&INPESC)
			gradient();
		else
			gradientWGamma();
		
		
		flushScreenAndWait();
		updateInput(mtime - prevTime);
		prevTime = mtime;
		_delay_ms(MSPT);
    }
}

