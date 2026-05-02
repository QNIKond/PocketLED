#include "input.h"
#include "common.h"
#include "UARTDebug.h"
#include <stdint.h>
#include <avr/io.h>

uint8_t inputRaw;
uint8_t inputDown;
uint8_t inputUp;

uint8_t prevInput;

#define DEBOUNCETIME 4 //32.768 ms
uint8_t timeouts[8];

void inputSetup(){
	DDRD |= _BV(PORTD5)| _BV(PORTD6);
}

//TODO: optimize
static void updateInputStates(uint8_t dt){
	inputDown = 0;
	inputUp = 0;
	for (uint8_t i = 0; i < 8; ++i){
		if (timeouts[i]< DEBOUNCETIME){
			timeouts[i] += dt;
		}
		if(((prevInput^inputRaw)&(1<<i))&&(timeouts[i] >= DEBOUNCETIME)){
			timeouts[i] = 0;
			if(inputRaw&(1<<i))
				inputDown |= (1<<i);
			else
				inputUp |= (1<<i);
		}
	}
}

void updateInput(uint8_t dt){
	TURNPON(D,6);
	inputRaw = 255;
	for (int i = 0; i < 8; ++i){
		inputRaw ^= ISPINP(D,7)>>i;
		BLINKP(D,5);
		
	}
	TURNPOFF(D,6);
	
	
	#ifdef STAND
		inputRaw &= 3;
	#else
		inputRaw ^= ISPINP(D,2)<<5;
	#endif // STAND

	inputRaw |= dbgKeyInput;
	updateInputStates(dt);
	prevInput = inputRaw;
}