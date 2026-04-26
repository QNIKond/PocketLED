#ifndef SOUND_H_
#define SOUND_H_
#include "common.h"
#include <stdint.h>

#define SLIDEDIRUP 1
#define DSLIDEDIRUP 2
typedef struct{
	//Frequency
	uint16_t freqStep; //freqStep * [half the number of modulation periods in one period] = 2^16
	uint16_t slide; //Current freqStep will be incremented by slide every period
	uint16_t dslide; //Current slide will be incremented by dslide every period
	uint8_t slideDirection;
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

extern uint8_t volume;

void soundSetup();

void playNote(const Note *note);

extern Note asdf;

#endif /* SOUND_H_ */