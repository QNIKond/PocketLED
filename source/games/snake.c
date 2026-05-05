#include "../os.h"
#include "../common.h"
#include "../graphics.h"
#include "../UARTDebug.h"
#include "../input.h"

#define SNUP 0b00
#define SNLEFT 0b10
#define SNDOWN 0b01
#define SNRIGHT 0b11

#define TICKSPEED 122

static struct{
	uint8_t dir;
	uint8_t length;
	uint8_t shape[64]; //64b
	uint8_t head;
	uint8_t headX;
	uint8_t headY;
	
	uint8_t tailX;
	uint8_t tailY;
	
	uint16_t field[16]; //32b
	
	uint8_t inpQueue;
	uint8_t iqLen;
	
	uint8_t tickT;
	
	uint8_t fruitX;
	uint8_t fruitY;
	
} *sd;

//#define FMOVE(X,Y, D) ((D)&2 ? (X) : (Y)) += (((D)&1)<<1) - 1;
#define FMOVE(X,Y, D) if((D)&0b10) (X) += (((D)&1)<<1) - 1; else (Y) += (((D)&1)<<1) - 1;
#define GETTAIL (sd->head+1<sd->length ? sd->head+1 : 0)

static void reset(){
	sendMsg(0x14);
	sd->dir = SNRIGHT;
	sd->length = 3;
	sd->shape[0] = 0b111111;
	sd->head = 2;
	sd->headX = 4;
	sd->headY = 7;
	sd->tailX = 2;
	sd->tailY = 7;
	
	for(uint8_t i = 0; i < 16; ++i)
		sd->field[i] = 0;
	sd->field[7] = 0b111<<2;
	
	sd->inpQueue = 0;
	sd->iqLen = 0;
	sd->tickT = 0;
	
	sd->fruitX = 12;
	sd->fruitY = 7;
}

void SnakeStart(void* mem){
	sd = mem;
	reset();
	//sendMsg(0x11);
}

static inline void updateDirection(){
	if(sd->iqLen >= 4)
	return;
	if(inputDown&INPUP)
	sd->inpQueue |= SNUP<<((sd->iqLen++)*2);
	if(inputDown&INPLEFT)
	sd->inpQueue |= SNLEFT<<((sd->iqLen++)*2);
	if(inputDown&INPDOWN)
	sd->inpQueue |= SNDOWN<<((sd->iqLen++)*2);
	if(inputDown&INPRIGHT)
	sd->inpQueue |= SNRIGHT<<((sd->iqLen++)*2);
}

#define SHAPECLEAR(X) sd->shape[(X)>>2] &= ~(0x03<<(((X)&0x03)<<1))
#define SHAPESET(X, V) sd->shape[(X)>>2] |= ((V)<<(((X)&0x03)<<1))
#define SHAPEGET(X) ((sd->shape[(X)>>2] >> (((X)&0x03)<<1))&3)

static inline void qCycle(){
	
	sd->head = GETTAIL;
	SHAPECLEAR(sd->head);
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

static inline void gameTick(){
	if(sd->iqLen){
		sd->dir = sd->inpQueue&0x03;
		sd->inpQueue >>= 2;
		--sd->iqLen;
	}
	
	uint8_t tx = sd->headX;
	uint8_t ty = sd->headY;
	FMOVE(tx,ty, sd->dir);
	if((tx>15)||(ty>15)||((sd->field[ty]>>tx)&1)){//
		reset();
		return;
	}
	sd->headX = tx;
	sd->headY = ty;
	
	sd->field[sd->headY] |= 1<<sd->headX;
	sd->field[sd->tailY] &= ~(1<<sd->tailX);
	
	uint8_t ttt = 0;
	for (uint8_t i = 0; i < 16; ++i){
		for(uint8_t j = 0; j < 16; ++j){
			ttt += (sd->field[i]>>j)&1;
		}
	}
	if(ttt!=sd->length)
		sendParam(0x15,ttt);
	
	
	
	if((sd->headX == sd->fruitX) && (sd->headY == sd->fruitY)){
		qAdd();
		do{
			sd->fruitX = xorshift32()&15;
			sd->fruitY = xorshift32()&15;
		} while (sd->field[sd->fruitY]>>sd->fruitX&1);
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
	if(inputUp&INPA){ //sd->tickT >= TICKSPEED
		sd->tickT = 0;
		gameTick();
	}
	drawSnake();
	
	
	if(inputUp&INPESC)
		SnakeStop();
}

static inline void drawSnake(){
	uint8_t cx = sd->headX;
	uint8_t cy = sd->headY;
	uint8_t h = sd->head;
	for(uint8_t i = 0; i <sd->length; ++i){
		canvas[cy][cx] = 255;
		
		uint8_t d = SHAPEGET(h);
		d ^= 1;
		FMOVE(cx,cy,d);
		if(--h > sd->length)
			h  = sd->length-1;
	}
	
	canvas[sd->fruitY][sd->fruitX] = GAMMA(128);
}

void SnakeStop(){
	osExitToMenu();
	
}

void SnakeDrawTitle(uint8_t dt){
	HLINE(6,6,9,255);
	HLINE(9,6,9,255);
}

GAMEIMPLEMENT(Snake)