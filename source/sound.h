#ifndef SOUND_H_
#define SOUND_H_
#include "common.h"
#include <stdint.h>

#define SLIDEDIRUP 1
#define DSLIDEDIRUP 2

#define MODFREQ 31250
#define MSTOMPERIOD(MS)				(MS##UL*MODFREQ/2000)
#define MSTOOVERFLOW(MS, C)			(uint16_t)(65535UL*(C)/MSTOMPERIOD(MS))
#define FREQTOSTEP(F)				((65536*(F)*2) / MODFREQ)

#define FREQSTEP(F)					.freqStep = FREQTOSTEP(F)
#define SLIDECURVE(MS, DF, DDF)		.slide = (FREQTOSTEP(DF)*16777216LL)/MSTOMPERIOD(MS),\
									.dslide = ((FREQTOSTEP(DDF)*16777216LL)/MSTOMPERIOD(MS))/MSTOMPERIOD(MS)
#define ATTACKCURVE(MS)				.attackTime = MSTOMPERIOD(MS), \
									.attackSlope = MSTOOVERFLOW(MS, 255)
#define SUSTAINCURVE(MS, FIN)		.sustainTime = MSTOMPERIOD(MS), \
									.sustainSlope = MSTOOVERFLOW(MS, 255-FIN)
#define DECAYCURVE(MS, START)		.decayTime = MSTOMPERIOD(MS), \
									.decaySlope = MSTOOVERFLOW(MS, START)
#define RETRIGGER(LOW, HIGH)		.lowRetrigger = FREQTOSTEP(LOW), \
									.highRetrigger = FREQTOSTEP(HIGH) \

typedef struct{
	//Frequency
	uint16_t freqStep; //freqStep * [half the number of modulation periods in one period] = 2^16
	int32_t slide; //Current freqStep will be incremented by slide every period
	int32_t dslide; //Current slide will be incremented by dslide every period
	uint16_t lowRetrigger; //When freqStep passes below lowRetrigger frequency is reset
	uint16_t highRetrigger; //When freqStep passes above highRetrigger frequency is reset
	
	//Amplitude
	uint16_t attackTime; //Number of periods
	uint16_t attackSlope; //Is added to current mq each period
	uint16_t sustainTime;
	uint16_t sustainSlope; //Is subtracted from current mq each period
	uint16_t decayTime;
	uint16_t decaySlope;
	
	//Misc
	uint8_t grain;	//How much noise is added
	uint8_t repeat; //0xFF Toggle mode
					//N Repeat N times after first one 
} Note;

extern const Note tone;
extern const Note rampup;
extern const Note allNote;

extern uint8_t volume;

void soundSetup();

void playNote(const Note *note, uint8_t priority);

void endNote();

extern Note asdf;

#endif /* SOUND_H_ */