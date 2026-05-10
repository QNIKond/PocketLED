#ifndef NOTES_H_
#define NOTES_H_
#include "sound.h"
#include <avr/pgmspace.h>

extern const Note tone;
extern const Note rampup;
extern const Note allNote;
extern const Note tnote1;
extern const Note tnote2;

#endif /*NOTES_H_*/
	
#ifdef NOTES_IMPLEMENTATION
#undef NOTES_IMPLEMENTATION
const Note tone PROGMEM = {
	FREQTOSTEP(1000),
	SUSTAINCURVE(1000, 255),
};
	
const Note rampup PROGMEM = {
	FREQTOSTEP(1000),
	ATTACKCURVE(500),
};

const Note allNote PROGMEM = {
	FREQTOSTEP(1000),
	SLIDECURVE(200, 400, 50),
	RETRIGGER(0, 2500),
	
	ATTACKCURVE(300),
	SUSTAINCURVE(300, 128),
	DECAYCURVE(300, 128),
	.grain = 200,
	.repeat = 1
};

const Note tnote1 PROGMEM = {
	FREQTOSTEP(800),
	DECAYCURVE(600, 255),
};

const Note tnote2 PROGMEM = {
	FREQTOSTEP(1400),
	DECAYCURVE(600, 255),
};

#endif