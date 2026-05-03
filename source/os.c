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

static uint8_t curGame = 1;
void mainMenu(uint8_t dt);
void (*running)(uint8_t dt) = mainMenu;

void osSetup(){
	cli();
	inputSetup();
	LEDMatrixSetup(&dt);
	UARTSetup();
	soundSetup();
	sei();
}

void mainMenu(uint8_t dt){
	static uint8_t textT = 0;
	static uint8_t textTCount = 0;
	static uint8_t isInnit = 0;
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
		curGame = (curGame-1)%GAMESCOUNT;
		textTCount = 0;
		textT = 0;
		isInnit = 0;
	}
	if (inputUp&INPRIGHT) {
		curGame = (curGame+1)%GAMESCOUNT;
		textTCount = 0;
		textT = 0;
		isInnit = 0;
	}
	if((inputUp&INPA)&&ISIMPLEMENTED(curGame)){
		textTCount = 0;
		textT = 0;
		isInnit = 0;
		games[curGame]->start();
		running = games[curGame]->update;
	}
	
	if(ISIMPLEMENTED(curGame))
		games[curGame]->drawTitle(dt);
		
	drawRunningTitle(canvas, textT, 0,
				gameNames[curGame].name,gameNames[curGame].len);
	
	//Draw Title separator			
	uint8_t temp = 1;
	for (uint8_t i = 0; i < 8; ++i){
		canvas[5][i] = temp;
		canvas[5][15-i] = temp;
		temp <<= 1;
	}
	
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
	for (uint8_t i = 0; i < 8; ++i){
		sendParam(0x15, GAMMA(1<<i));
	}
	while (1){
		running(dt);
		flushScreenAndWait();
		updateInput(dt);
		dt = 1;
	}
}