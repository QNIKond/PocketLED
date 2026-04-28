#include "sound.h"
#include "common.h"
#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "UARTDebug.h"

Note curNote;
uint8_t curPriority;

uint16_t freqCount;
fl32 curFreqStep;

uint8_t curMq; 
uint16_t amplCount;
uint16_t curSlope; //Added to amplCount each period

fl32 curSlide;

uint16_t mperiodCounter;
uint16_t targetMPeriodCount;

uint8_t curStage;
uint8_t repeats;

void soundSetup(){
	DDRB |= 1 << PORTB2;
	TCCR1A |= _BV(COM1B1);//Set on Bottom, clear on match
	TCCR1A |= _BV(WGM11); //Fast PWM, TOP: 0x01FF
	TCCR1B |= _BV(WGM12);
	//TCCR1B |= _BV(CS10); //Prescaler 1
	OCR1BL = 3;
	OCR1A = 0xFFFF;
	TIMSK1 |= _BV(TOIE1);
}

static inline void startBlankHalfPeriod(uint8_t t){
	TCCR1A &= ~_BV(WGM11); //Normal mode
	TCCR1B &= ~_BV(WGM12);
	
	TCCR1C |= _BV(FOC1B); //Clearing pin
	
	TCCR1B &= ~_BV(CS10);
	TCCR1B |= _BV(CS12); //Prescaler 256
	OCR1B = t;
	
	TIMSK1 |= _BV(OCIE1B); //Enabling compare A interrupt
	TIFR1 |= _BV(TOV1); //Clearing overflow interrupt flag
	TCNT1 = 0;
}

static inline void startMainHalfPeriod(){
	TCCR1A |= _BV(WGM11);
	TCCR1B |= _BV(WGM12);
	
	TCCR1B &= ~_BV(CS12);
	TCCR1B |= _BV(CS10);
	
	TIMSK1 &= ~_BV(OCIE1B);
}

static inline void loadCurNote(const Note *note){
	memcpy_P(&curNote, note,sizeof(curNote));
}

static inline void resetFrequency(){
	FLTOINT16(curFreqStep) = curNote.freqStep;
	
	curSlide = (uint32_t)curNote.slide<<16;
	freqCount = 0;
}

static inline void resetSound(){
	resetFrequency();
	curMq = 3;
	amplCount = 0;
	curSlope = curNote.attackSlope;
	targetMPeriodCount = curNote.attackTime;
	mperiodCounter = 0;
	curStage = 0;
	repeats = 0;
	
	OCR1BL = 3;
}

void playNote(const Note *note, uint8_t priority){
	if(curPriority >= priority)
		return;
	
	curPriority = priority;
	loadCurNote(note);
	resetSound();
	startMainHalfPeriod();
	
}

void endNote(){
		sendParam(0x16, (curFreqStep)>>24);
		sendParam(0x15, (curFreqStep)>>16);

	curPriority = 0;
	TCCR1B &= ~_BV(CS10);
	TCCR1B &= ~_BV(CS12);
	TCCR1C |= _BV(FOC1B); //Clearing pin
	TIFR1 |= _BV(TOV1); //Clearing overflow interrupt flag
	TIFR1 |= _BV(OCF1B); //Clearing overflow interrupt flag
}

static inline void updateFrequency(uint8_t t){
	if(curNote.slideDirection&SLIDEDIRUP) curFreqStep += ((curSlide)>>11)*t;
	else curFreqStep -= ((curSlide)>>11)*t;
	if(curNote.slideDirection&DSLIDEDIRUP) curSlide += curNote.dslide*t;
	else curSlide -= curNote.dslide*t;
	
// 	sendParam(0x15, (curSlide)>>16);
// 	sendParam(0x16, (curSlide)>>24);
	
	if((curNote.lowRetrigger && (FLTOINT16(curFreqStep) <= curNote.lowRetrigger)) ||
		(curNote.highRetrigger && (FLTOINT16(curFreqStep) >= curNote.highRetrigger)))
		resetFrequency();
}

static inline void nextStage(){
	++curStage;
	mperiodCounter = 0;
	if(curStage == 1){
		curMq = 255;
		targetMPeriodCount = curNote.sustainTime;
		curSlope = curNote.sustainSlope;
	}
	else if (curStage == 2){
		targetMPeriodCount = curNote.decayTime;
		curSlope = curNote.decaySlope;
	}
	else{
		++repeats;
		
		if((curNote.repeat != 0xFF) && (repeats >= curNote.repeat+1)){
			
			endNote();
		}
		else{
			uint8_t t = repeats;
			resetSound();
			repeats = t;
		}
	}
}

//Called every 512 instructions (31250Hz)
uint16_t ttt;
ISR(TIMER1_OVF_vect){
	//sei();
	freqCount += FLTOINT16(curFreqStep);
	if(freqCount < FLTOINT16(curFreqStep)){ //End of half period
		//retrieveParam(0x11, 0x02, OCR1BL);
// 		sendParam(0x15, FLTOINT16(curFreqStep)>>8);
// 		sendParam(0x16, FLTOINT16(curFreqStep));
		uint16_t hps = 65535UL/FLTOINT16(curFreqStep) + 1;
		mperiodCounter += hps;
		updateFrequency(hps);
		startBlankHalfPeriod(hps);
		
		if(mperiodCounter >= targetMPeriodCount){
			
			nextStage();
		}
	}
	
	amplCount += curSlope;
	if(amplCount < curSlope){
		if(!curStage)
			++curMq;
		else
			--curMq;
	}
	//cli();
}

ISR(TIMER1_COMPB_vect){
	startMainHalfPeriod();
	if(curNote.grain)
		OCR1BL = lerp(0, curMq, 255-lerp(0, (uint8_t)xorshift32(), curNote.grain));
	else
		OCR1BL = curMq;
	if(OCR1BL < 3) //OCR1BL can be equal to 0 but not 1 and 2?
		OCR1BL = 3;
}