#include "../os.h"
#include "../common.h"
#include "../graphics.h"
#include "../UARTDebug.h"
#include "../input.h"
#include <avr/pgmspace.h>

typedef struct  {
	uint8_t type;
	v2 shape[3];
	v2 pos;
	uint8_t rot;
} Tetro;

struct{
	uint8_t tickCount;
	
	uint8_t col[3*10*2]; // col[0]&1 is at the top  60b
	Tetro curT; //10b
	Tetro nextT;
	Tetro holdT;
	uint8_t holdEnabled;
	
} *td; //71b (72b)

enum{
	JTETRO=1,//111111
	LTETRO,//111111
	STETRO,//1111111111
	TTETRO,//11111111
	ZTETRO,//111111
	ITETRO,//1111111
	OTETRO,//111111
};

const uint8_t shapes[] PROGMEM = {	-1,-1,-1, 0, 1, 0,
									-1, 0, 1, 0, 1,-1,
									-1 ,0, 0,-1, 1,-1,
									-1, 0, 0,-1, 1, 0,
									-1,-1, 0,-1, 1, 0,
									-1, 0, 1, 0, 2, 0,
									 0,-1, 1, 0, 1,-1
									};

const uint8_t kicks[] PROGMEM = {
					 0, 0, 0, 0, 0, 0, 0, 0,
					 0, 0, 1, 0, 0, 0,-1, 0,
					 0, 0, 1, 1, 0, 0,-1, 1,
					 0, 0, 0,-2, 0, 0, 0,-2,
					 0, 0, 1,-2, 0, 0,-1,-2,

					 0, 0,-1, 0,-1,-1, 0,-1,
					-1, 0, 0, 0, 1,-1, 0,-1,
					 2, 0, 0, 0,-2,-1, 0,-1,
					-1, 0, 0,-1, 1, 0, 0, 1,
					 2, 0, 0, 2,-2, 0, 0,-2,

					 0, 0, 0, 1,-1, 1,-1, 0,
					 0, 0, 0, 1,-1, 1,-1, 0,
					 0, 0, 0, 1,-1, 1,-1, 0,
					 0, 0, 0, 1,-1, 1,-1, 0,
					 0, 0, 0, 1,-1, 1,-1, 0
					};

//const uint8_t Tcolors[8] = {0,64,116,144,172,200,228,255};
const uint8_t Tcolors[8] = {0,50,72,108,144,180,216,252};
#define TCGLOWDOWN 40

#define TETRISTICKSPEED 80

#define TKICK(X)

static void spawnNextT();
static void resetTetris(){
	td->tickCount = 0;
 	for (uint8_t i = 0; i < 60; ++i)
 		td->col[i] = 0;
	spawnNextT();
	spawnNextT();
	td->holdT.type = 0;
}

void TetrisStart(void* mem){
	td = mem;
	resetTetris();
}

static void setPoint(uint8_t x, uint8_t y, uint8_t c){
	if((x>9)||(y>15))
		return;
	uint8_t shift = (x<<1)+((y&8)>>3);
	td->col[shift] |= (c&1)<<(y&7);
	td->col[shift+20] |= ((c&2)>>1)<<(y&7);
	td->col[shift+40] |= ((c&4)>>2)<<(y&7);
}

static uint8_t testPointCollision(uint8_t x, uint8_t y){
	if(y>20)
		return 0;
	if((x>9)||(y>15))
		return 1;
	uint8_t shift = (x<<1)+((y&8)>>3);
	uint8_t f = td->col[shift] | td->col[shift+20] | td->col[shift+40];
	return (f>>(y&7))&1;
}

static uint8_t testCollision(Tetro *t){
	if(testPointCollision(t->pos.x, t->pos.y))
		return 1;
	for (uint8_t i = 0; i < 3; ++i){
		uint8_t x = t->pos.x + t->shape[i].x;
		uint8_t y = t->pos.y + t->shape[i].y;
		if(testPointCollision(x,y))
			return 1;
	}
	return 0;
}

static inline void moveT(){
	Tetro temp = td->curT;
	if (inputDown&INPLEFT){
		--temp.pos.x;
		if(testCollision(&temp))
			return;
	}
	else if (inputDown&INPRIGHT){
		++temp.pos.x;
		if(testCollision(&temp))
			return;
	}
	if ((inputDown&INPA) || (inputDown&INPB)){
		uint8_t rdir = inputDown&INPA ? -1 : 1;
		temp.rot = (temp.rot+rdir)&3;
		for (uint8_t i = 0; i < 3; ++i){
			temp.shape[i].x = td->curT.shape[i].y * (-rdir);
			temp.shape[i].y = td->curT.shape[i].x * (rdir);
		}
		uint8_t offset = 0;
		uint8_t oddOffset = 0;
		if (td->curT.type == ITETRO)
			oddOffset = 40;
		else if(td->curT.type == OTETRO)
		oddOffset = 80;
		
		do{
			if((offset == 5))
				return;
			
			temp.pos.x = td->curT.pos.x - pgm_read_byte(&kicks[(temp.rot<<1)+(offset<<3)+oddOffset]) + 
											pgm_read_byte(&kicks[(td->curT.rot<<1)+(offset<<3)+oddOffset]);
			temp.pos.y = td->curT.pos.y - pgm_read_byte(&kicks[(temp.rot<<1)+(offset<<3)+1+oddOffset]) +
											pgm_read_byte(&kicks[(td->curT.rot<<1)+(offset<<3)+1+oddOffset]);
			++offset;			
		} while (testCollision(&temp));
	}
	td->curT = temp;
}

static inline void removeRow(uint8_t y){
	uint16_t *cc = (uint16_t*)td->col;
	uint16_t mask = (1<<y)-1;
	for (uint8_t i = 0; i < 30; ++i)
		cc[i] = ((cc[i]&mask)<<1) | (cc[i]&((~mask)<<1));
	
}

static inline void testRows(){
	uint8_t rhigh = 0xFF;
	uint8_t rlow = 0xFF;
	for (uint8_t i = 0; i < 10; ++i){
		rhigh &= td->col[i<<1] | td->col[(i<<1)+20] | td->col[(i<<1)+40];
		rlow &= td->col[(i<<1)+1] | td->col[(i<<1)+21] | td->col[(i<<1)+41];
	}
	uint8_t y = 0;
	while(rhigh){
		if (rhigh&1)
			removeRow(y);
		rhigh >>= 1;
		++y;
	}
	y = 8;
	while(rlow){
		if (rlow&1)
		removeRow(y);
		rlow >>= 1;
		++y;
	}
}

static uint8_t glide;
static inline void fallT(){
	++td->curT.pos.y;
	if(testCollision(&td->curT)){
		--td->curT.pos.y;
		if(!glide)
			glide = 1;
		else{
			if(td->curT.pos.y>20){
				resetTetris();
				return;
			}
			setPoint(td->curT.pos.x, td->curT.pos.y, td->curT.type);
			for (uint8_t i = 0; i < 3; ++i){
				uint8_t y = (td->curT.pos.y + td->curT.shape[i].y);
				if(y>20){
					resetTetris();
					return;
				}
				setPoint(	td->curT.pos.x + td->curT.shape[i].x,
				y,
				td->curT.type);
			}
			testRows();
			spawnNextT();
		}
	}
}

static void buildTShape(Tetro *tetro){
	for (uint8_t i = 0; i < 3; ++i){
		uint8_t addr = 6*(tetro->type-1)+(i<<1);
		tetro->shape[i] = (v2){pgm_read_byte(&shapes[addr]),pgm_read_byte(&shapes[addr+1])};
	}
}

static void spawnNextT(){
	td->curT = td->nextT;
	do{ 
		td->nextT.type = ((uint8_t)xorshift32())&7;
	}while (!td->nextT.type);
	
	buildTShape(&td->nextT);
	
	td->nextT.pos = (v2){13, 14};
	td->curT.pos = (v2){4, 0};
	td->curT.rot = 0;
	
	td->holdEnabled = 1;
}

static inline void drawTetris(uint8_t dt);
void TetrisStop();
void TetrisUpdate(uint8_t dt){
	if(inputDown)
		moveT();
	if((inputDown&INPUP) && td->holdEnabled){
		
		uint8_t temp = td->curT.type;
		if(td->holdT.type){
			td->curT = td->holdT;
			td->curT.pos = (v2){4, 0};
			td->curT.rot = 0;
		}
		else{
			spawnNextT();
		}
		td->holdT.type = temp;
		buildTShape(&td->holdT);
		td->holdT.pos = (v2){13, 9};
		td->holdEnabled = 0;
	}
	if(inputUp&INPDOWN){//++td->tickCount > TETRISTICKSPEED
		td->tickCount = 0;
		fallT();
	}
	drawTetris(dt);
	if(inputUp&INPESC)
		TetrisStop();
}

static void drawTetro(Tetro *tetro, uint8_t xbound){
	uint8_t y = tetro->pos.y;
	uint8_t x =  tetro->pos.x;
	if((x<xbound)&&(y<16))
		canvas[y][x] = GAMMA(Tcolors[tetro->type]);
	for (uint8_t i = 0; i < 3; ++i){
		y = tetro->pos.y+tetro->shape[i].y;
		x = tetro->pos.x+tetro->shape[i].x;
		if((x<xbound)&&(y<16))
			canvas[y][x] = GAMMA(Tcolors[tetro->type]);
	}
}

static inline void drawTetris(uint8_t dt){
	for (uint8_t i = 0; i < 20; ++i){
		uint8_t c1 = td->col[i];
		uint8_t c2 = td->col[i+20];
		uint8_t c3 = td->col[i+40];
		for (uint8_t j = 0; j < 8; ++j){
			uint8_t type = (c1&1) | ((c2&1)<<1) | ((c3&1)<<2);
			canvas[j+((i&1)<<3)][i>>1] = GAMMA(Tcolors[type]);
				
			c2 >>= 1;
			c3 >>= 1;
			c1 >>= 1;
		}
	}
	drawTetro(&td->curT,10);
	drawTetro(&td->nextT,16);
	if (td->holdT.type){
		drawTetro(&td->holdT,16);
	}
	VLINE(10, 7, 15, 2);
	HLINE(11, 10, 15, 2);
	HLINE(6, 10, 15, 2);
}

void TetrisStop(){
	osExitToMenu();
}

void TetrisDrawTitle(uint8_t dt){
	
}

GAMEIMPLEMENT(Tetris)