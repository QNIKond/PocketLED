#include "sound.h"
#include <avr/pgmspace.h>
/*
	//Amplitude
	uint16_t attackTime; //Number of periods
	fl16 attackSlope; //Is added to current mq each period
	uint16_t sustainTime;
	fl16 sustainSlope; //Is subtracted from current mq each period
	uint16_t decayTime;
	fl16 decaySlope;
	
	uint8_t grain; //How much noise is added
	
	//Frequency
	uint8_t mperiod; //Half of the number of samples in one modulated signal period
	fl16 slide; //Current mperiod will be incremented by slide
	fl16 dslide; //Current slide will be incremented by dslide
	uint8_t lowRetrigger; //When mperiod passes below lowRetrigger frequency is reset
	uint8_t highRetrigger; //When mperiod passes above highRetrigger frequency is reset
	
	//Misc
	uint8_t repeat; //0 Toggle mode
	//N Repeat N times*/

#define MODFREQ 31250
#define FREQTOMPERIOD(X) (MODFREQ/X)/2
#define MSTOPERIOD(X) 

const Note tone PROGMEM = {
	.mperiod = 12,
	.sustainTime = 12,
	.repeat = 1
	};
	
const Note rampup PROGMEM = {
	.attackTime = 12,
	.attackSlope = 12,
	.mperiod = 12,
	.repeat = 1
};