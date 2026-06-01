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
#include "save.h"

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
uint8_t gamesCount = (sizeof(games)/sizeof(Game*));

static uint8_t curGame = 0;
void updateMainMenu(uint8_t dt);
void (*running)(uint8_t dt);

void resetMainMenu();
void osSetup(){
	cli();
	inputSetup();
	LEDMatrixSetup(&dt);
	UARTSetup();
	soundSetup();
	saveSetup();
	
	resetMainMenu();
	running = updateMainMenu;
	sei();
}

static void startGame(Game *g, uint8_t arg){
	running = g->update;
	*(g->memory) = &__heap_start;
	if(!restoreGame(g, &__heap_start))
		arg |= OSRESET;
	g->start(arg);
}

uint8_t isTransitioning;
Game *trnGame;
#define TRNTIME_MS 300
#define TRNSTEP 512000UL*256/(122UL*TRNTIME_MS)
uint16_t curTrnTime;
uint8_t trnDir;
uint8_t startArgs;

static void startTransition(Game *game, uint8_t arg){
	if(isTransitioning)
		return;
	isTransitioning = 1;
	trnGame = game;
	curTrnTime = 0;
	trnDir = 0;
	startArgs = arg;
}

static void updateTransition(uint8_t dt){
	curTrnTime += TRNSTEP;
	if(curTrnTime < TRNSTEP){
		
		if(trnDir){
			isTransitioning = 0;
			curTrnTime = 255<<8;
		}
		else{
			startGame(trnGame, startArgs);
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
	*(games[curGame]->tcmemory) = (uint8_t*)((uint16_t)&__heap_start + games[curGame]->memSize);
	games[curGame]->resetTitle();
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
	
	if (inpUpEvent&INPLEFT) {
		playNote(&N_dbeep800, 128, FREQSTEP(1400));
		curGame =(curGame+gamesCount-1)%gamesCount;
		resetMainMenu();
	}
	if (inpUpEvent&INPRIGHT) {
		playNote(&N_dbeep800, 128);
		curGame = (curGame+1)%gamesCount;
		resetMainMenu();
	}
	if((inpUpEvent&INPA)){
		playNote(&N_enter, 192);
		startTransition(games[curGame], 0);
	}
	if(inpHoldEvent&INPA){
		playNote(&N_enter, 192);
		if(inpIsPressed&INPB)
			startTransition(games[curGame], OSHARDRESET);
		else
			startTransition(games[curGame], OSRESET);
	}
		
	
	xorshift32();
	
	
	games[curGame]->drawTitle(dt);
	
	
	for(uint8_t i = 0; i < 16; ++i)
		for(uint8_t j = 0; j < TITLEMINHEIGHT; ++j)
			canvas[j][i] = 0;
			
	for(uint8_t i = 0; i < 16; ++i)
		canvas[15][i] = 0;
	
	drawRunningTitle(textT, 0,
				gameNames[curGame].name,gameNames[curGame].len);
	shadeScreen(5);
	
// 	//Draw Title separator			
// 	uint8_t temp = 1;
// 	for (uint8_t i = 0; i < 8; ++i){
// 		canvas[5][i] = temp;
// 		canvas[5][15-i] = temp;
// 		temp <<= 1;
// 	}
	
	//Draw bottom navigation dots
	uint8_t mx = 0;
	for (uint8_t i = 0; i < gamesCount; ++i){
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
	while (1){
		running(dt);
		if(isTransitioning)
			updateTransition(dt);
		flushScreenAndWait();
		updateInput(dt);
		dt = 1;
		if (((inpDownEvent&INPUP) && (inpIsPressed&INPDOWN))||
		((inpDownEvent&INPDOWN) && (inpIsPressed&INPUP))){
			isMuted ^= 1;
		}
		if (((inpDownEvent&INPB) && (inpIsPressed&INPDOWN))||
		((inpDownEvent&INPDOWN) && (inpIsPressed&INPB))){
			eraseSave(games[curGame]);
			DPOINT4;
		}
		
		if(dbgFlags&(1<<0x02)){
			dbgFlags &= ~(1<<0x02);
			osSaveAndExit();
		}
	}
}

void osSaveAndExit(){
	saveGame(games[curGame]);
	resetMainMenu();
	running = updateMainMenu;
}
