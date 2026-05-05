#include "graphics.h"
#include "common.h"
#include <stdint.h>
#include <avr/pgmspace.h>
#include "UARTDebug.h"

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
255};

const uint16_t font[] PROGMEM = {
	0x0000,
	 
	0xF6DE, 0x4924, 0xE7CE, 0xF3CE, 0xB792, 0xF39E, 0xF3DE, 0xE492, 0xF7DE, 0xF79E,
	 
	0xF7DA, 0xD7DC, 0x7246, 0xD6DC, 0xF3CE, 0xF3C8, 0xF25E, 0xB7DA, 0xE92E, 0xE92C, 0xB75A, 0x924E, 0xBEDA, 
	0xBFFA, 0xF6DE, 0xF7C8, 0xF6E6, 0xF7EA, 0xF39E, 0xE924, 0xB6DE, 0xB6D4, 0xB6FA, 0xB55A, 0xB524, 0xE54E, 
	
	0x76B0, 0x9EF0, 0x7230, 0x3EF0, 0xFE70, 0x6BA0, 0xF2F0, 0x93D0, 0x4120, 0x4160, 0xBAD0, 0x4910, 0xBED0, 
	0xD6D0, 0x56A0, 0xF7C0, 0xF790, 0x7240, 0x71F0, 0xE920, 0xB6B0, 0xB6A0, 0xB7D0, 0xAAD0, 0xB520, 0xE570
	};

void gradientScreen(){
	for(uint8_t i = 0; i < 16; ++i)
		for(uint8_t j = 0; j < 16; ++j)
			canvas[i][j] = (i<<4) + j;
}


void gradientWGammaScreen(){
	for(uint8_t i = 0; i < 16; ++i)
		for(uint8_t j = 0; j < 16; ++j)
			canvas[i][j] = GAMMA((i<<4) + j);
}

//const uint8_t shade[8] = {256,254,248,240,224,192,128,64};
//const uint8_t cshade[8] = {52,58,69,84,106,136,182,255};
const uint8_t bshade[8] = {128, 192, 255, 255, 255, 255, 255, 255};
void drawLetter(int8_t x, int8_t y, uint8_t letter){
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
			if(!(((x+i)|(y+j))&240) && (bl&0x8000)){
				if ((x+i)<8) 
					canvas[y+j][x+i] = GAMMA(bshade[x+i]);
				else 
					canvas[y+j][x+i] = GAMMA(bshade[15-x-i]);
			}
			bl<<=1;
		}
	}
}

// t>>2 < len
void drawRunningText(uint8_t t, int8_t y, const char* str, uint8_t len){
	int8_t lc = -(t&3);
	const char* s = str + (t>>2);
	while((lc<16)){
		drawLetter(lc, y, *(s++));
		lc += 4;
		if(!(*s)) s = str;
	}
}

void drawRunningTitle(uint8_t t, int8_t y, const char* str, uint8_t len){
	uint8_t divT = t / 5;
	int8_t lc = -(t-divT*5);
	const char* s = str + (divT);
	while((lc<16)){
		drawLetter(lc, y, (*(s++))&(~0x20));
		lc += 5;
		if(!(*s)) s = str;
	}
}