#include "graphics.h"
#include "common.h"
#include <stdint.h>
#include <avr/pgmspace.h>
#include "UARTDebug.h"

const uint16_t font[] PROGMEM = {
	0x0000,
	 
	0xF6DE, 0x4924, 0xE7CE, 0xF3CE, 0xB792, 0xF39E, 0xF3DE, 0xE492, 0xF7DE, 0xF79E,
	 
	0xF7DA, 0xD7DC, 0x7246, 0xD6DC, 0xF3CE, 0xF3C8, 0xF25E, 0xB7DA, 0xE92E, 0xE92C, 0xB75A, 0x924E, 0xBEDA, 
	0xBFFA, 0xF6DE, 0xF7C8, 0xF6E6, 0xF7EA, 0xF39E, 0xE924, 0xB6DE, 0xB6D4, 0xB6FA, 0xB55A, 0xB524, 0xE54E, 
	
	0x76B0, 0x9EF0, 0x7230, 0x3EF0, 0xFE70, 0x6BA0, 0xF2F0, 0x93D0, 0x4120, 0x4160, 0xBAD0, 0x4910, 0xBED0, 
	0xD6D0, 0x56A0, 0xF7C0, 0xF790, 0x7240, 0x71F0, 0xE920, 0xB6B0, 0xB6A0, 0xB7D0, 0xAAD0, 0xB520, 0xE570
	};

void drawLetter(uint8_t (*sc)[16], int8_t x, int8_t y, uint8_t letter){
	uint8_t l = 0;
	uint16_t bl = 0;
	
	if(letter&0x40)
		l = letter&0x20 ? 36+(letter&31) : 10+(letter&31);
	else if(letter&0x10)
		l = 1+(letter&15);
	
	HIGH8(bl) = pgm_read_byte(&HIGH8(font[l]));
	bl |= pgm_read_byte(&font[l]);
	
	for (uint8_t j = 0; j < 5; ++j){
		for (uint8_t i = 0; i < 3; ++i){
			if(!(((x+i)|(y+j))&240) && (bl&0x8000))
				sc[y+j][x+i] = 255;
			bl<<=1;
		}
	}
}

// t>>2 < len
void drawRunningText(uint8_t (*sc)[16], uint8_t t, int8_t y, const char* str, uint8_t len){
	int8_t lc = -(t&3);
	const char* s = str + (t>>2);
	while((lc<16)){
		drawLetter(sc,lc, y, *(s++));
		lc += 4;
		if(!(*s)) s = str;
	}
}
