#include "sound.h"
#include "common.h"
#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

Note curNote;

uint8_t curHps; //half period samples counter
uint16_t freqCount;
uint16_t curFreqStep;
fl16 curMq; //Current length of the modulation impulse in cpu ticks
fl16 curSlope; //Added to curMq each period

//fl16 curMperiod;
uint16_t curSlide;

uint16_t periodCounter;
uint8_t targetPeriodCount;

uint8_t curStage;
uint8_t repeats;

void soundSetup(){
	TCCR1A |= _BV(COM1B1);//Set on Bottom, clear on match
	TCCR1A |= _BV(WGM11); //Fast PWM, TOP: 0x01FF
	TCCR1B |= _BV(WGM12);
	TCCR1B |= _BV(CS10); //Prescaler 1
	OCR1AL = 3;
	OCR1B = 0xFFFF;
	TIMSK1 |= _BV(TOIE1);
}

static inline void startBlankHalfPeriod(uint8_t t){
	TCCR1A &= ~_BV(WGM11); //Normal mode
	TCCR1B &= ~_BV(WGM12);
	
	TCCR1C |= _BV(FOC1B); //Clearing pin
	
	TCCR1B &= ~_BV(CS10);
	TCCR1B |= _BV(CS12); //Prescaler 256
	OCR1A = t;
	
	TIMSK1 |= _BV(OCIE1A); //Enabling compare A interrupt
	TIFR1 |= _BV(TOV1); //Clearing overflow interrupt flag
}

static inline void startMainHalfPeriod(){
	TCCR1A |= _BV(WGM11);
	TCCR1B |= _BV(WGM12);
	
	TCCR1B &= ~_BV(CS12);
	TCCR1B |= _BV(CS10);
	
	TIMSK1 &= ~_BV(OCIE1A);
}

static inline void loadCurNote(const Note *note){
	memcpy_P(&curNote, note,sizeof(curNote));
}

static inline void resetFrequency(){
	curFreqStep = curNote.freqStep;
	curSlide = curNote.slide;
}

void playNote(const Note *note){
	loadCurNote(note);
	resetFrequency();
	curHps = 0;
	curMq = (fl16)(3<<8);
	curSlope = curNote.attackSlope;
	periodCounter = 0;
	targetPeriodCount = curNote.attackTime;
	curStage = 0;
	repeats = 0;
	
	OCR1AL = 3;
	startMainHalfPeriod();
}

static inline void endNote(){
	TCCR1B &= ~_BV(CS10);
}

static inline void updateFrequency(){
	if((curFreqStep+curSlide)>1)
		curFreqStep += curSlide;
	curSlide += curNote.dslide;
	
	if((curNote.lowRetrigger && (curFreqStep <= curNote.lowRetrigger)) ||
		(curNote.highRetrigger && (curFreqStep >= curNote.highRetrigger)))
		resetFrequency();
}

static inline void updateAmplitude(){
	++curStage;
	if(curStage == 1){
		targetPeriodCount += curNote.sustainSlope;
		curSlope = curNote.sustainSlope;
	}
	else if (curStage == 2){
		targetPeriodCount += curNote.decayTime;
		curSlope = curNote.decaySlope;
	}
	else{
		++repeats;
		if(curNote.repeat && (repeats >= curNote.repeat))
			endNote();
		else{
			curStage = 0;
			
			periodCounter = 0;
			resetFrequency();
			
			targetPeriodCount = curNote.attackTime;
			curSlope = curNote.attackSlope;
		}
	}
}

//Called every 512 instructions (31250Hz)
ISR(TIMER1_OVF_vect){
	++curHps;
	freqCount += curFreqStep;
	if(freqCount < curFreqStep){ //End of half period
		curMq += curSlope;
		startBlankHalfPeriod(curHps);
		curHps = 0;
		updateFrequency();
		if(++periodCounter >= targetPeriodCount)
			updateAmplitude();
	}
}

ISR(TIMER1_COMPA_vect){
	startMainHalfPeriod();
	if(curNote.grain)
		OCR1AL = lerp(0, FLTOINT8(curMq), 255-lerp(0, (uint8_t)xorshift32(), curNote.grain));
	else
		OCR1AL = FLTOINT8(curMq);
	if(OCR1AL < 3) //OCR1AL can be equal to 0 but not 1 and 2?
		OCR1AL = 3;
}