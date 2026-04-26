#include "sound.h"
#include <avr/pgmspace.h>
/*
	//Frequency
	uint16_t freqStep; //freqStep * [half the number of modulation periods in one period] = 2^16
	uint16_t slide; //Current freqStep will be incremented by slide every period
	uint16_t dslide; //Current slide will be incremented by dslide every period
	uint8_t slideDirection;
	uint16_t lowRetrigger; //When freqStep passes below lowRetrigger frequency is reset
	uint16_t highRetrigger; //When freqStep passes above highRetrigger frequency is reset
	
	//Amplitude
	uint16_t attackTime; //Number of periods. Total time should be less than 2 sec.
	fl16 attackSlope; //Is added to current mq each period
	uint16_t sustainTime;
	fl16 sustainSlope;//Is subtracted from current mq each period
	uint16_t decayTime;
	fl16 decaySlope;
	
	//Misc
	uint8_t grain;	//How much noise is added
	uint8_t repeat; //0 Toggle mode
					//N Repeat N times*/


#define MODFREQ 31250
#define MSTOMPERIOD(MS) (MS##UL*MODFREQ/1000) 
#define MSTOOVERFLOW(MS, C) (uint16_t)(65535UL*C/MSTOMPERIOD(MS))
#define FREQTOSTEP(F)				((65536*F*2) / MODFREQ)
#define FREQSTEP(F)					.freqStep = FREQTOSTEP(F)
#define SLIDECURVE(MS, DF, DDF, S)	.slide = (FREQTOSTEP(DF)*2048UL)/MSTOMPERIOD(MS),\
									.dslide = (FREQTOSTEP(DDF)*2048UL)/MSTOMPERIOD(MS),\
									.slideDirection = S
#define ATTACKCURVE(MS)				.attackTime = MSTOMPERIOD(MS), \
									.attackSlope = MSTOOVERFLOW(MS, 255)
#define SUSTAINCURVE(MS, FIN)		.sustainTime = MSTOMPERIOD(MS), \
									.sustainSlope = MSTOOVERFLOW(MS, 255-FIN)
#define DECAYCURVE(MS, START)		.decayTime = MSTOMPERIOD(MS), \
									.decaySlope = MSTOOVERFLOW(MS, START)
#define RETRIGGER(LOW, HIGH)		.lowRetrigger = FREQTOSTEP(LOW), \
									.highRetrigger = FREQTOSTEP(HIGH) \
								
const Note tone PROGMEM = {
	FREQTOSTEP(1000),
	SUSTAINCURVE(1000, 255),
};
	
const Note rampup PROGMEM = {
	FREQTOSTEP(1000),
	ATTACKCURVE(500),
};

const Note fullTest PROGMEM = {
	FREQTOSTEP(1000),
	SLIDECURVE(200, 400, 50,  SLIDEDIRUP),
	RETRIGGER(0, 2500),
	
	ATTACKCURVE(300),
	SUSTAINCURVE(300, 128),
	DECAYCURVE(300, 128),
	.grain = 200,
	.repeat = 1
};