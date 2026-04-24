#include "LEDMatrix.h"
#include "common.h"
#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>

static uint8_t screen2[16][16];
static uint8_t screen1[16][16];
static uint8_t* currentlyDisplayed = (uint8_t*)screen1;
static uint8_t* curOp = (uint8_t*)screen1;
uint8_t (*canvas)[16] = screen2;
static volatile uint8_t isNewFrame;

const uint8_t _NeoPixelGammaTable[256] PROGMEM = { //_NeoPixelGammaTable
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   1,
	1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   2,   2,   2,   3,
	3,   3,   3,   3,   3,   4,   4,   4,   4,   5,   5,   5,   5,   5,   6,
	6,   6,   6,   7,   7,   7,   8,   8,   8,   9,   9,   9,   10,  10,  10,
	11,  11,  11,  12,  12,  13,  13,  13,  14,  14,  15,  15,  16,  16,  17,
	17,  18,  18,  19,  19,  20,  20,  21,  21,  22,  22,  23,  24,  24,  25,
	25,  26,  27,  27,  28,  29,  29,  30,  31,  31,  32,  33,  34,  34,  35,
	36,  37,  38,  38,  39,  40,  41,  42,  42,  43,  44,  45,  46,  47,  48,
	49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
	64,  65,  66,  68,  69,  70,  71,  72,  73,  75,  76,  77,  78,  80,  81,
	82,  84,  85,  86,  88,  89,  90,  92,  93,  94,  96,  97,  99,  100, 102,
	103, 105, 106, 108, 109, 111, 112, 114, 115, 117, 119, 120, 122, 124, 125,
	127, 129, 130, 132, 134, 136, 137, 139, 141, 143, 145, 146, 148, 150, 152,
	154, 156, 158, 160, 162, 164, 166, 168, 170, 172, 174, 176, 178, 180, 182,
	184, 186, 188, 191, 193, 195, 197, 199, 202, 204, 206, 209, 211, 213, 215,
	218, 220, 223, 225, 227, 230, 232, 235, 237, 240, 242, 245, 247, 250, 252,
255};uint64_t mtime; //Race condition possible
void LEDMatrixSetup(){
	TCCR0A = 0x02; //CTC mode
	OCR0A = 64;
	TCCR0B = 0x02; //Prescaler 8
	TIMSK0 = 0x02; //Enable A match interrupt
	
	DDRB |= _BV(PORTB0) | _BV(PORTB1);
	DDRC |= 0b00111111;
	
	DDRD |= _BV(PORTD3) | _BV(PORTD4);
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
	uint8_t t = PORTB & _BV(PORTB2);
	PORTB = *curOp | t;
	PORTC = (*curOp) >> 2;
	++curOp;
}

//8.192 ms per frame
ISR(TIMER0_COMPA_vect){
	OCR0A >>= 1;
	if(OCR0A <= 3){
		drawCurRow(); //... | 4*8 cycles | this row | 2*8 cycles | ...
		_delay_loop_2(4); //(2*8 - 8)/4
		drawCurRow(); //... | 2*8 cycles | this row | 1*8 cycles | ...

		PORTB = 0;
		PORTC = 0;
		
		if(curOp >= currentlyDisplayed+8*32){
			TURNPON(D,3);
			BLINKP(D,4);
			TURNPOFF(D,3);
			//updateInput(); //????????
			++mtime;
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
