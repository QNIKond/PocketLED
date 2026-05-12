#ifndef NOTES_H_
#define NOTES_H_
#include "sound.h"
#include <avr/pgmspace.h>

extern const Note N_dbeep800;
extern const Note N_dbeep1400;
extern const Note allNote;
extern const Note N_enter;

#endif /*NOTES_H_*/
	
#ifdef NOTES_IMPLEMENTATION
#undef NOTES_IMPLEMENTATION
const Note N_dbeep800 PROGMEM = {
	FREQTOSTEP(800),
	DECAYCURVE(300, 255),
};
const Note N_dbeep1400 PROGMEM = {
	FREQTOSTEP(1400),
	DECAYCURVE(300, 255),
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

const Note N_enter PROGMEM = {
	FREQTOSTEP(800),
	SLIDECURVE(600, 2000, -2000),
	RETRIGGER(0, 1200),
	
	ATTACKCURVE(100),
	DECAYCURVE(500, 255),
};

#endif