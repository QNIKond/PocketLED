#include "LEDMatrix.h"
#include "common.h"
#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include "UARTDebug.h"

static uint8_t screen2[16][16];
static uint8_t screen1[16][16];
static uint8_t* currentlyDisplayed = (uint8_t*)screen1;
static uint8_t* curOp = (uint8_t*)screen1;
static volatile uint8_t isNewFrame;static volatile uint8_t* fc;uint8_t (*canvas)[16] = screen2;

void LEDMatrixSetup(volatile uint8_t* frameCount){
	TCCR0A = 0x02; //CTC mode
	OCR0A = 64;
	TCCR0B = 0x02; //Prescaler 8
	TIMSK0 = 0x02; //Enable A match interrupt
	
	DDRB |= _BV(PORTB0) | _BV(PORTB1);
	DDRC |= 0b00111111;
	
	DDRD |= _BV(PORTD3) | _BV(PORTD4);
	
	fc = frameCount;
}
static inline uint64_t TransposeFlip(uint64_t x){
	uint64_t t;
	
	t = (x ^ (x >> 7))  & 0x00AA00AA00AA00AAULL;
	x ^= t ^ (t << 7);

	t = (x ^ (x >> 14)) & 0x0000CCCC0000CCCCULL;
	x ^= t ^ (t << 14);

	t = (x ^ (x >> 28)) & 0x00000000F0F0F0F0ULL;
	x ^= t ^ (t << 28);
	
	x = ((x >> 8)  & 0x00FF00FF00FF00FFULL) | ((x & 0x00FF00FF00FF00FFULL) << 8);
	x = ((x >> 16) & 0x0000FFFF0000FFFFULL) | ((x & 0x0000FFFF0000FFFFULL) << 16);
	x = (x >> 32) | (x << 32);
	
	return x;
}

#define SC64  ((uint64_t*)sc)
#define PREVINCH(X) ((X)&16 ? ((X)<<1)-31 : (X)<<1)
void prepareScreen(uint8_t* sc){
	uint8_t chains[] = {0,1,3,5,7,11,23,31};
	for(int i = 0; i < 8; ++i){
		uint8_t curind = chains[i];
		uint64_t first = TransposeFlip(SC64[curind]);
		uint8_t prevind = PREVINCH(curind);
		while(prevind != chains[i]){
			SC64[curind] = TransposeFlip(SC64[prevind]);
			curind = prevind;
			prevind = PREVINCH(curind);
		}
		SC64[curind] = first;
	}
}
static inline void drawCurRow(){
	PORTB = *curOp&(~_BV(PORTB2));
	PORTC = (*curOp) >> 2;
	++curOp;
}

//8.192 ms per frame (122.07 FPS)
ISR(TIMER0_COMPA_vect){
	OCR0A >>= 1;
	if(OCR0A <= 7){
// 		drawCurRow();
// 		_delay_loop_2(64);
		drawCurRow();
		_delay_loop_2(16);
		drawCurRow();
		_delay_loop_2(4);
		drawCurRow();
		PORTB = 0;
		PORTC = 0;
		
		if(curOp >= currentlyDisplayed+8*32){
			TURNPON(D,3);
			BLINKP(D,4);
			TURNPOFF(D,3);
			++(*fc);
			if(isNewFrame){
				isNewFrame = 0;
				uint8_t* t = currentlyDisplayed;
				currentlyDisplayed = (uint8_t*)canvas;
				canvas = (uint8_t(*)[16])t;
			}
			curOp = currentlyDisplayed;
		}
		else{
			BLINKP(D,4);
		}
		
		OCR0A = 255;
		TCNT0 = 0;
		TIFR0 |=2;
	}
	else
		TCNT0 = 0;
	drawCurRow();
}
inline void flushScreenAndWait(){
	while(isNewFrame);
	prepareScreen((uint8_t*)canvas);
	isNewFrame = 1;
	while(isNewFrame);
	memset(canvas, 0, 16*16);
}
