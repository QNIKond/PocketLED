#include "os.h"
#include "common.h"
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "LEDMatrix.h"
#include "input.h"
#include "UARTDebug.h"
#include "sound.h"
#include "notes.h"
#include "graphics.h"

static volatile uint8_t dt;

#define X(A) &A,
Game *games[] = {XGAMES};
#undef X
#define X(Y) {#Y " ",sizeof(#Y)},
struct{
	const char* name;
	uint8_t len;
} gameNames[] = {XGAMES};
#undef X

static uint8_t curGame = 0;
void updateMainMenu(uint8_t dt);
void (*running)(uint8_t dt);

void osSetup(){
	cli();
	inputSetup();
	LEDMatrixSetup(&dt);
	UARTSetup();
	soundSetup();
	sei();
}

uint8_t isTransitioning;
void (*dest)(uint8_t dt);
void (*destStart)(void* mem);
#define TRNTIME_MS 300
//#define TRNSTEP 32000*256/122*TRNTIME_MS
#define TRNSTEP 512000UL*256/(122UL*TRNTIME_MS)
uint16_t curTrnTime;
uint8_t trnDir;

static void startTransition(void (*d)(uint8_t dt), void (*ds)(void* mem)){
	if(isTransitioning)
		return;
	isTransitioning = 1;
	dest = d;
	destStart = ds;
	curTrnTime = 0;
	trnDir = 0;
}

static void updateTransition(uint8_t dt){
	curTrnTime += TRNSTEP;
	if(curTrnTime < TRNSTEP){
		
		if(trnDir){
			isTransitioning = 0;
			curTrnTime = 255<<8;
		}
		else{
			running = dest;
			destStart(&__heap_start);
			trnDir = 1;
			curTrnTime = 0;
		}
	}
	uint8_t maxc;
	if(trnDir)
		maxc = 255-HIGH8(curTrnTime);
	else
		maxc = HIGH8(curTrnTime);
	uint8_t c = maxc;
	uint8_t cy = 15;
	while(c>16){
		for (uint8_t i = 0; i < 16; ++i)
			canvas[cy][i] = lerp(canvas[cy][i], c, maxc);
		--cy;
		c -= 16;
	}
	
}

static uint8_t textT;
static uint8_t textTCount;
static uint8_t isInnit;

void resetMainMenu(){
	textT = 0;
	textTCount = 0;
	isInnit = 0;
	games[curGame]->resetTitle(&__heap_start);
}

void updateMainMenu(uint8_t dt){
	
	textTCount += dt;
	
	if (!isInnit){
		if(textTCount>=TEXTINNIT){
			textTCount = 0;
			isInnit = 1;
		}
	}
	else if (textTCount >= TEXTSCROLLSPEED){
		textTCount = 0;
		++textT;
		if (textT >= gameNames[curGame].len*5) textT = 0;
	}
	
	if (inputUp&INPLEFT) {
		playNote(&N_dbeep800, 128, FREQSTEP(1400));
		curGame = (curGame-1)%GAMESCOUNT;
		resetMainMenu();
	}
	if (inputUp&INPRIGHT) {
		playNote(&N_dbeep800, 128);
		curGame = (curGame+1)%GAMESCOUNT;
		resetMainMenu();
	}
	if((inputUp&INPA)){
		playNote(&N_enter, 192);
		startTransition(games[curGame]->update, games[curGame]->start);
		//running = games[curGame]->update;
	}
	
	xorshift32();
	
	
	games[curGame]->drawTitle(dt);
		
	drawRunningTitle(textT, 0,
				gameNames[curGame].name,gameNames[curGame].len);
	
// 	//Draw Title separator			
// 	uint8_t temp = 1;
// 	for (uint8_t i = 0; i < 8; ++i){
// 		canvas[5][i] = temp;
// 		canvas[5][15-i] = temp;
// 		temp <<= 1;
// 	}
	
	//Draw bottom navigation dots
	uint8_t mx = 0;
	for (uint8_t i = 0; i < GAMESCOUNT; ++i){
		if (i == curGame){
			canvas[15][mx] = 255;
			canvas[15][mx+1] = 255;
			mx += 3;
		} 
		else{
			canvas[15][mx] = GAMMA(96);
			mx += 2;
		}
	}
}

void osRun(){
	osExitToMenu();
	while (1){
		running(dt);
		if(isTransitioning)
			updateTransition(dt);
		flushScreenAndWait();
		updateInput(dt);
		dt = 1;
		if (((inputDown&INPUP) && (inputRaw&INPDOWN))||
		((inputDown&INPDOWN) && (inputRaw&INPUP))){
			isMuted ^= 1;
		}
		
		if(dbgFlags&(1<<0x02)){
			dbgFlags &= ~(1<<0x02);
			osExitToMenu();
		}
	}
}

void osExitToMenu(){
	resetMainMenu();
	running = updateMainMenu;
}
