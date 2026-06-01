#include "input.h"
#include "common.h"
#include "UARTDebug.h"
#include <stdint.h>
#include <avr/io.h>

uint8_t inpIsPressed;
uint8_t inpDownEvent;
uint8_t inpUpEvent;
uint8_t inpIsHolding;
uint8_t inpHoldEvent;

uint8_t prevInput;

#define DEBOUNCETIME 4 //32.768 ms
#define HOLDTIME 150
uint8_t timeouts[8];

void inputSetup(){
	DDRD |= _BV(PORTD5)| _BV(PORTD6);
}


static void updateInputStates(uint8_t dt){
	inpDownEvent = 0;
	inpUpEvent = 0;
	inpHoldEvent = 0;
	uint8_t upd = prevInput^inpIsPressed;
	uint8_t cur = 1;
	for(uint8_t i = 0; i < 8; ++i){
		if(timeouts[i]<HOLDTIME)
			timeouts[i] += dt;
		else if(inpDownEvent&cur){
			if(!(inpIsHolding&cur))
				inpHoldEvent |= cur;
			inpIsHolding |= cur;
		}
		if((cur&upd) & (timeouts[i] >= DEBOUNCETIME)){
			timeouts[i] = 0;
			if(inpIsPressed&(1<<i))
				inpDownEvent |= (1<<i);
			else{
				inpUpEvent |= (1<<i);
				inpIsHolding &= ~cur;
			}
		}
	
		cur <<= 1;
	}
	prevInput = inpIsPressed;
}

void updateInput(uint8_t dt){
	TURNPON(D,6);
	inpIsPressed = 255;
	for (int i = 0; i < 8; ++i){
		inpIsPressed ^= ISPINP(D,7)>>i;
		BLINKP(D,5);
		
	}
	TURNPOFF(D,6);
	
	
	#ifdef STAND
		inpIsPressed &= 3;
	#else
		inpIsPressed |= 1<<7;
		inpIsPressed ^= ISPINP(D,2)<<5;
	#endif // STAND

	inpIsPressed |= dbgKeyInput;
	updateInputStates(dt);
}