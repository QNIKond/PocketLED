#include "../os.h"
#include "../common.h"
#include "../graphics.h"
#include "../UARTDebug.h"
#include "../input.h"

#define SNUP 0b00
#define SNLEFT 0b10
#define SNDOWN 0b01
#define SNRIGHT 0b11

uint8_t debugSpeed = 30;
#define SNAKETICKSPEED debugSpeed

static struct{
	uint8_t dir;
	uint8_t length;
	uint8_t shape[64]; //64b
	uint8_t head;
	uint8_t headX;
	uint8_t headY;
	
	uint8_t tailX;
	uint8_t tailY;
	
	//uint16_t field[16]; //32b
	
	uint8_t inpQueue;
	uint8_t iqLen;
	
	uint8_t tickT;
	
	uint8_t fruitX;
	uint8_t fruitY;
	
	uint8_t invFrame;
	
	uint8_t colorStep;
} *sd;

//#define FMOVE(X,Y, D) ((D)&2 ? (X) : (Y)) += (((D)&1)<<1) - 1;
#define FMOVE(X,Y, D) if((D)&0b10) (X) += (((D)&1)<<1) - 1; else (Y) += (((D)&1)<<1) - 1;
#define GETTAIL (sd->head+1<sd->length ? sd->head+1 : sd->head+1-sd->length)

static void reset(){
	sd->dir = SNRIGHT;
	sd->length = 3;
	sd->shape[0] = 0b111111;
	sd->head = 2;
	sd->headX = 4;
	sd->headY = 7;
	sd->tailX = 2;
	sd->tailY = 7;
	
	sd->inpQueue = 0;
	sd->iqLen = 0;
	sd->tickT = 0;
	
	sd->fruitX = 12;
	sd->fruitY = 7;
	
	sd->invFrame = 0;
	
	sd->colorStep = 255/sd->length;
}

void SnakeStart(void* mem){
	sd = mem;
	reset();
	//sendMsg(0x11);
}

#define GETLASTDIR (sd->iqLen ? (sd->inpQueue>>(sd->iqLen-1))&3 : sd->dir)
static inline void updateDirection(){
	
		uint8_t dir;
	if(inputDown&INPUP)
		dir = SNUP;
	else if(inputDown&INPLEFT)
		dir = SNLEFT;
	else if(inputDown&INPDOWN)
		dir = SNDOWN;
	else if(inputDown&INPRIGHT)
		dir = SNRIGHT;
	else
		return;

	if((sd->iqLen >= 4) || ((dir&2) == (GETLASTDIR&2)))
		return;
	
	sd->inpQueue |= dir<<((sd->iqLen++)*2);
}

#define SHAPESET(X, V) do {sd->shape[(X)>>2] &= ~(0x03<<(((X)&0x03)<<1));\
							sd->shape[(X)>>2] |= ((V)<<(((X)&0x03)<<1));}while(0)
#define SHAPEGET(X) ((sd->shape[(X)>>2] >> (((X)&0x03)<<1))&3)

static inline void qCycle(){
	
	sd->head = GETTAIL;
	SHAPESET(sd->head, sd->dir);
}

static inline void qAdd(){
	if(sd->length >255) return;
	for(uint8_t i = sd->length; i > sd->head+1; --i){
		SHAPESET(i,SHAPEGET(i-1));
	}
	++sd->length;
	//qCycle();
}

static uint8_t testSnakeCollision(uint8_t x, uint8_t y){
	uint8_t cx = sd->headX;
	uint8_t cy = sd->headY;
	uint8_t h = sd->head;
	for(uint8_t i = 0; i <sd->length; ++i){
		if((cx==x)&&(cy==y))
			return 1;
		uint8_t d = SHAPEGET(h);
		d ^= 1;
		FMOVE(cx,cy,d);
		
		if(h == 0)
			h  = sd->length-1;
		else
			--h;
	}
	return 0;
}

static inline void gameTick(){
	if(sd->iqLen){
		sd->dir = sd->inpQueue&0x03;
		sd->inpQueue >>= 2;
		--sd->iqLen;
	}
	
	uint8_t tx = sd->headX;
	uint8_t ty = sd->headY;
	FMOVE(tx,ty, sd->dir);
	if((tx>15)||(ty>15)||testSnakeCollision(tx,ty)){//
		if(!sd->invFrame)
			sd->invFrame = 1;
		else
			reset();
		return;
	}
	sd->headX = tx;
	sd->headY = ty;

	if((sd->headX == sd->fruitX) && (sd->headY == sd->fruitY)){
		do{
			sd->fruitX = xorshift32()&15;
			sd->fruitY = xorshift32()&15;
		} while (testSnakeCollision(sd->fruitX, sd->fruitY));
		qAdd();
		sd->colorStep = 255/sd->length;
	}
	else{
		uint8_t tail = (sd->head+2<sd->length ? sd->head+2 : sd->head+2-sd->length);
		tail = SHAPEGET(tail);
		FMOVE(sd->tailX,sd->tailY, tail);
	}
	qCycle();
}

void SnakeStop();
static inline void drawSnake();
void SnakeUpdate(uint8_t dt){
	updateDirection();
	sd->tickT += dt;
	if(sd->tickT >= SNAKETICKSPEED){ //sd->tickT >= TICKSPEED
		sd->tickT = 0;
		gameTick();
	}
	drawSnake();
	
	DUPDATE1(debugSpeed);
	
	if(inputUp&INPESC)
		SnakeStop();
}

static inline void drawSnake(){
	uint8_t cx = sd->headX;
	uint8_t cy = sd->headY;
	uint8_t h = sd->head;
	uint8_t col = 255;
	for(uint8_t i = 0; i <sd->length; ++i){
		if((cx>15)||(cy>15)||(canvas[cy][cx] == 255))
			DPOINT1;
		canvas[cy][cx] = GAMMA(col);
		col -= sd->colorStep;
		uint8_t d = SHAPEGET(h);
		d ^= 1;
		FMOVE(cx,cy,d);
		if(h == 0)
			h  = sd->length-1;
		else
			--h;
	}
	
	canvas[sd->fruitY][sd->fruitX] = GAMMA(128);
}

void SnakeStop(){
	osExitToMenu();
	
}

struct{
	uint8_t waves[8];
	int8_t ttail;
	uint8_t tickCount;
} *stcd; //58b
#define WAVESCOUNT (sizeof(stcd->waves)/sizeof(uint8_t))
#define TITLESNAKELEN 16
#define TITLESNAKETICKSPEED 15

void SnakeResetTitle(void* tmem){
	stcd = tmem;
	for (uint8_t i = 0; i < WAVESCOUNT; ++i){
		stcd->waves[i] = (xorshift32()&3)+1;
	}
	stcd->ttail = xorshift32()&15;
	stcd->tickCount = 0;
}

#define DRAWIFSNAKE if((curind>stcd->ttail) && (curind<stcd->ttail + TITLESNAKELEN))\
					canvas[cury][curx] = GAMMA(((curind-stcd->ttail)<<4));
void SnakeDrawTitle(uint8_t dt){
	uint8_t curx = 0;
	uint8_t cury = 10;
	int8_t curind = 0;
	uint8_t curwave = 0;
	int8_t dir = 1;
	while ((curwave<8) && (curind < stcd->ttail + TITLESNAKELEN)){
		DRAWIFSNAKE;
		for (uint8_t i = 0; i < stcd->waves[curwave]; ++i){
			cury+=dir;
			++curind;
			DRAWIFSNAKE;
		}
		for (uint8_t i = 0; (i < 2)&&(curx<15); ++i){
			++curx;
			++curind;
			DRAWIFSNAKE;
		}
		for (uint8_t i = 0; (i < stcd->waves[curwave])&&(curx<15); ++i){
			cury-=dir;
			++curind;
			DRAWIFSNAKE;
		}
		++curwave;
		dir = -dir;
	}
	if(++stcd->tickCount > TITLESNAKETICKSPEED){
		stcd->tickCount = 0;
		++stcd->ttail;
		if(stcd->ttail > curind){
			stcd->ttail = -TITLESNAKELEN;
			for (uint8_t i = 0; i < WAVESCOUNT; ++i){
				stcd->waves[i] = (xorshift32()&3)+1;
			}
		}
	}
	
}

GAMEIMPLEMENT(Snake)