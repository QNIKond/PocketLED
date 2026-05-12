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

static uint8_t textT;
static uint8_t textTCount;
static uint8_t isInnit;

void startMainMenu(){
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
		playNote(&N_dbeep1400, 128);
		curGame = (curGame-1)%GAMESCOUNT;
		startMainMenu();
	}
	if (inputUp&INPRIGHT) {
		playNote(&N_dbeep800, 128);
		curGame = (curGame+1)%GAMESCOUNT;
		startMainMenu();
	}
	if((inputUp&INPA)){
		playNote(&N_enter, 192);
		games[curGame]->start(&__heap_start);
		running = games[curGame]->update;
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
	startMainMenu();
	running = updateMainMenu;
}
