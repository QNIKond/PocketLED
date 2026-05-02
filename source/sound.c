#include "sound.h"
#include "common.h"
#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "UARTDebug.h"

static Note curNote;
static uint8_t curPriority;

static uint16_t freqCount;
static fl32 curFreqStep;//FF FF . FF FF
static fl32 curSlide;//FF. FF FF FF

static uint8_t curMq; 
static uint16_t amplCount;
static uint16_t curSlope; //Added to amplCount each period


static uint16_t mperiodCounter;
static uint16_t targetMPeriodCount;

static uint8_t curStage;
static uint8_t repeats;

static uint16_t hps;

void soundSetup(){
	OCR1A = 0xFFFF;
	TIMSK1 |= _BV(TOIE1);
}

static inline void enablePin(){
	DDRB |= _BV(PORTB2);
	TCCR1A |= _BV(COM1B1);
}

static inline void disablePin(){
	TCCR1A &= ~_BV(COM1B1);
	DDRB &= ~_BV(PORTB2);
	PORTB &= ~_BV(PORTB2);
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
	TIFR1 |= _BV(OCF1A);
	TCNT1 = 0;
}

static inline void startMainHalfPeriod(){
	TCCR1A |= _BV(WGM11);
	TCCR1B |= _BV(WGM12);
	
	TCCR1B &= ~_BV(CS12);
	TCCR1B |= _BV(CS10);
	
	TIMSK1 &= ~_BV(OCIE1B);
	
	TIFR1 |= _BV(TOV1); //Clearing overflow interrupt flag
	TIFR1 |= _BV(OCF1A);
	TCNT1 = 0;
}

static inline void resetFrequency(){
	FLTOINT16(curFreqStep) = curNote.freqStep;
	
	curSlide = curNote.slide;
	freqCount = 0;
	hps = 0;
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
	enablePin();
	if(curPriority >= priority)
		return;
	
	curPriority = priority;
	memcpy_P(&curNote, note,sizeof(curNote));
	resetSound();
	startMainHalfPeriod();
	
}

void endNote(){
	curPriority = 0;
	TCCR1B &= ~_BV(CS10);
	TCCR1B &= ~_BV(CS12);
	TIFR1 |= _BV(TOV1); //Clearing overflow interrupt flag
	TIFR1 |= _BV(OCF1B); //Clearing overflow interrupt flag
	disablePin();
}


static inline void updateFrequency(uint8_t t){
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
ISR(TIMER1_OVF_vect){
	sei();
	freqCount += FLTOINT16(curFreqStep);
	curFreqStep += (((uint32_t)curSlide+0x80000000)>>8) - 0x800000;
	curSlide += curNote.dslide;
	++hps;
	if(freqCount < FLTOINT16(curFreqStep)){ //End of half period
		mperiodCounter += hps;
		//cli();
		updateFrequency(hps);
		startBlankHalfPeriod(hps);
		hps = 0;
		if(mperiodCounter >= targetMPeriodCount){
			nextStage();
		}
	}
	
	amplCount += curSlope;
	if(amplCount < curSlope){
		if(curStage)
			--curMq;
		else
			++curMq;
	}
	cli();
}

ISR(TIMER1_COMPB_vect){
	sei();
	startMainHalfPeriod();
	if(curNote.grain)
		OCR1BL = lerp(0, curMq, 255-lerp(0, (uint8_t)xorshift32(), curNote.grain));
	else
		OCR1BL = curMq;
	if(OCR1BL < 3)
		OCR1BL = 3;
	cli();
}