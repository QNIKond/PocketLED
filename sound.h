#ifndef SOUND_H_
#define SOUND_H_
#include "common.h"
#include <stdint.h>

typedef struct{
	//Amplitude
	uint16_t attackTime; //Number of periods
	fl16 attackSlope; //Is added to current mq each period
	uint16_t sustainTime;
	fl16 sustainSlope; //Is subtracted from current mq each period
	uint16_t decayTime;
	fl16 decaySlope;
	
	uint8_t grain; //How much noise is added
	
	//Frequency
	//uint8_t mperiod; //Half of the number of samples in one modulated signal period
	uint16_t freqStep; //freqStep * [half the number of modulation periods in one period] = 2^16
	uint16_t slide; //Current freqStep will be incremented by slide
	uint16_t dslide; //Current slide will be incremented by dslide
	uint16_t lowRetrigger; //When freqStep passes below lowRetrigger frequency is reset
	uint16_t highRetrigger; //When freqStep passes above highRetrigger frequency is reset
	
	//Misc
	uint8_t repeat; //0 Toggle mode
					//N Repeat N times
} Note;

extern const Note tone;
extern const Note rampup;

void soundSetup();

void playNote(const Note *note);

extern Note asdf;

#endif /* SOUND_H_ */