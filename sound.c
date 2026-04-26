#include "sound.h"
#include "common.h"
#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

Note curNote;

uint16_t freqCount;
fl32 curFreqStep;

uint8_t curMq; 
uint16_t amplCount;
uint16_t curSlope; //Added to amplCount each period

uint16_t curSlide;

uint16_t mperiodCounter;
uint8_t targetMPeriodCount;

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
	FLTOINT16(curFreqStep) = curNote.freqStep;
	curSlide = curNote.slide;
	freqCount = 0;
}

static inline void resetSound(){
	resetFrequency();
	curMq = 3;
	amplCount = 0;
	curSlope = curNote.attackSlope;
	mperiodCounter = 0;
	targetMPeriodCount = curNote.attackTime;
	curStage = 0;
	repeats = 0;
	
	OCR1AL = 3;
}

void playNote(const Note *note){
	loadCurNote(note);
	resetSound();
	startMainHalfPeriod();
}

static inline void endNote(){
	TCCR1B &= ~_BV(CS10);
	TCCR1C |= _BV(FOC1B); //Clearing pin
}

static inline void updateFrequency(uint8_t t){
	if(curNote.slideDirection&SLIDEDIRUP) curFreqStep += ((uint32_t)curSlide)<<5*t;
	else curFreqStep -= curSlide<<5*t;
	if(curNote.slideDirection&DSLIDEDIRUP) curSlide += curNote.dslide;
	else curSlide -= curNote.dslide;
	
	if((curNote.lowRetrigger && (FLTOINT16(curFreqStep) <= curNote.lowRetrigger)) ||
		(curNote.highRetrigger && (FLTOINT16(curFreqStep) >= curNote.highRetrigger)))
		resetFrequency();
}

static inline void nextStage(){
	++curStage;
	if(curStage == 1){
		targetMPeriodCount += curNote.sustainSlope;
		curSlope = curNote.sustainSlope;
	}
	else if (curStage == 2){
		targetMPeriodCount += curNote.decayTime;
		curSlope = curNote.decaySlope;
	}
	else{
		++repeats;
		if((curNote.repeat+1) && (repeats >= curNote.repeat+1))
			endNote();
		else{
			uint8_t t = repeats;
			resetSound();
			repeats = t;
		}
	}
}

//Called every 512 instructions (31250Hz)
ISR(TIMER1_OVF_vect){
	freqCount += FLTOINT16(curFreqStep);
	if(freqCount < FLTOINT16(curFreqStep)){ //End of half period
		uint8_t hps = 255/FLTOINT16(curFreqStep) + 1;
		mperiodCounter += hps;
		updateFrequency(hps);
		startBlankHalfPeriod(hps);
		if(mperiodCounter >= targetMPeriodCount)
			nextStage();
	}
	
	amplCount += curSlope;
	if(amplCount < curSlope){
		if(!curStage)
			++curMq;
		else
			--curMq;
	}
}

ISR(TIMER1_COMPA_vect){
	startMainHalfPeriod();
	if(curNote.grain)
		OCR1AL = lerp(0, curMq, 255-lerp(0, (uint8_t)xorshift32(), curNote.grain));
	else
		OCR1AL = curMq;
	if(OCR1AL < 3) //OCR1AL can be equal to 0 but not 1 and 2?
		OCR1AL = 3;
}