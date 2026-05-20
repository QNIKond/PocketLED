#ifndef NOTES_H_
#define NOTES_H_
#include "sound.h"
#include <avr/pgmspace.h>

extern const Note N_dbeep800;
extern const Note N_dbeep1400;
extern const Note allNote;
extern const Note N_enter;
extern const Note N_enter_2;

extern const Note N_Smove;
extern const Note N_eat;
extern const Note N_death;
extern const Note N_Tmove;
extern const Note N_rotate;
extern const Note N_fastFall;
extern const Note N_hardDrop;
extern const Note N_softDrop;

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
	.chain = &allNote
};

const Note N_enter PROGMEM = {
	FREQTOSTEP(1500),
	SLIDECURVE(600, 2000, -2000),
	RETRIGGER(0, 2000),
	
	ATTACKCURVE(300),
	DECAYCURVE(100, 255),
	.chain = &N_enter_2,
};

const Note N_enter_2 PROGMEM = {
	FREQTOSTEP(800),
	SLIDECURVE(600, 2000, -2000),
	RETRIGGER(0, 1200),
	
	ATTACKCURVE(100),
	DECAYCURVE(500, 255),
};

const Note N_Smove PROGMEM = {
	FREQTOSTEP(100),
	ATTACKCURVE(100),
	DECAYCURVE(150, 255),
	
};

const Note N_eat PROGMEM = {//sounds like powerup
	FREQTOSTEP(800),
	SLIDECURVE(400, 1000, 0),
	SUSTAINCURVE(150,180),
	DECAYCURVE(300, 180),
};

const Note N_death PROGMEM = {
	FREQTOSTEP(1000),
	SLIDECURVE(650, -2000, -2000),
	RETRIGGER(100, 0),
	SUSTAINCURVE(150,180),
	DECAYCURVE(500, 180),
	.grain = 128
};

const Note N_Tmove PROGMEM = {
	FREQTOSTEP(100),
	SLIDECURVE(200, -100,100),
	DECAYCURVE(200, 255),
};

const Note N_rotate PROGMEM = {
	FREQTOSTEP(300),
	SLIDECURVE(350, 0,1000),
	ATTACKCURVE(250),
	DECAYCURVE(100, 255),
	.grain = 128
};

const Note N_fastFall PROGMEM = {
	FREQTOSTEP(1000),
	SLIDECURVE(1500, 200,0),
	ATTACKCURVE(1500),
};

const Note N_hardDrop PROGMEM = {
	FREQTOSTEP(1000),
	//SLIDECURVE(1500, 200,0),
	DECAYCURVE(1000, 255),
	.grain = 255
};
const Note N_softDrop PROGMEM = {
	FREQTOSTEP(600),
	//SLIDECURVE(1500, 200,0),
	DECAYCURVE(400, 255),
	.grain = 255
};

#endif