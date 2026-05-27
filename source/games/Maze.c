#include <stdint.h>
#include "../os.h"
#include "../common.h"
#include "../graphics.h"
#include "../input.h"
#include "../sound.h"
#include "../notes.h"
#include "../UARTDebug.h"
#include <avr/pgmspace.h>

// OOOOO
// O O O
//OO OOO

struct{
	uint8_t xWalls[8*8];
	uint8_t yWalls[8*8];
	uint8_t zWalls[8*8];
	v3 player;
	uint8_t atimer;
	uint8_t depth;
} *md; //197b

#define NVIS	0b000
#define DIRX	0b010
#define DIRNX	0b011
#define DIRY	0b100
#define DIRNY	0b101
#define DIRZ	0b110
#define DIRNZ	0b111

#define SETPOINT(G, X, Y, Z) G[(Y) + ((Z)<<3)] |= 1 << (X)
#define GETPOINT(G, X, Y, Z) ((G[(Y) + ((Z)<<3)]>>(X))&1)
#define GETPOINTV(G, V) ((G[(V.y) + ((V.z)<<3)]>>(V.x))&1)
#define PUSHDIR(G, GC, D) do{\
	G[(GC)>>1] &= GC&1 ? 0x0F : 0xF0;\
	G[(GC)>>1] |= D << (GC&1 ? 4 : 0);\
	++GC;}while(0)
#define POPDIR(G, GC, D) do{--GC; D = (G[(GC)>>1] >> (GC&1 ? 4 : 0))&15;}while(0)
#define MOVEFORWARD(V, D) \
		do{if(D&4){if(D&2) V.z += 1-((D&1)<<1); else V.y += 1-((D&1)<<1);}\
		else V.x += 1-((D&1)<<1);}while(0)
#define MOVEBACK(V, D) do{D^=1; MOVEFORWARD(V,D);}while(0)

static inline void getNextUnvisited(uint8_t *conf, v3 *p){
	while(conf[p->y + (p->z<<3)]==255){
		if(++p->y == 8){
			p->y = 0;
			++p->z;
			if(p->z >= md->depth)
				return;
		}
	}
	p->x = 0;
	for(uint8_t row = conf[p->y + (p->z<<3)];row&1;row>>=1)
		++p->x;
}

static inline uint32_t getDirections(uint8_t *visited, v3 *head, uint8_t *count){
	const uint8_t xshift[6] = {1,-1,0,0,0,0};
	const uint8_t yshift[6] = {0,0,1,-1,0,0};
	const uint8_t zshift[6] = {0,0,0,0,1,-1};
	uint32_t dirs = 0;
	for (uint8_t i = 0; i < 6; ++i){
		v3 v = (v3){
		.x = head->x + xshift[i],
		.y = head->y + yshift[i],
		.z = head->z + zshift[i]};
		if((v.x<8) && (v.y<8) && (v.z<md->depth) && !GETPOINTV(visited, v)){
			++(*count);
			dirs <<= 4;
			dirs |= i+2;
		}
	}
	return dirs;
}

static inline void breakWall(v3 p, uint8_t dir){
	uint8_t shift = dir&1 ? 0 : 1;
	if(dir&4){
		if(dir&2)
			SETPOINT(md->zWalls, p.x,p.y,p.z-shift);
		else
			SETPOINT(md->yWalls, p.x,p.y-shift,p.z);
	}
	else
		SETPOINT(md->xWalls, p.x-shift,p.y,p.z);
}

static inline void nextTrace(uint8_t *conf, v3 tail){
	uint8_t trace[8*8*4];
	uint8_t visited[8*8];
	uint16_t traceCount = 0;
	v3 head = tail;
	for(uint16_t i = 0; i < sizeof(trace); ++i)
		trace[i] = 0;
	for(uint8_t i = 0; i < sizeof(visited); ++i)
		visited[i] = 0;
	
	while(!GETPOINTV(conf, head)){
		SETPOINT(visited, head.x, head.y, head.z);
		uint8_t dcount = 0;
		uint32_t dirs = getDirections(visited, &head, &dcount);
		if(dcount){
			uint8_t rdirind = xorshift32()%dcount;
			uint8_t rdir = (dirs>>(rdirind<<2))&7;
			MOVEFORWARD(head, rdir);
			PUSHDIR(trace, traceCount, rdir);
		}
		else{
			if(!traceCount){
				return;
			}
			uint8_t dir;
			POPDIR(trace, traceCount, dir);
			MOVEBACK(head, dir);
		}
	}
	while(traceCount){
		uint8_t dir;
		POPDIR(trace, traceCount, dir);
		breakWall(head, dir);
		MOVEBACK(head, dir);
		SETPOINT(conf, head.x, head.y, head.z);
	}
	
}

static void resetMaze(){
	md->player = (v3){0,0,0};
	uint8_t confirmed[8*8];
	v3 tail;

	for(uint8_t i = 0; i < 64; ++i){
		md->xWalls[i] = 0;
		md->yWalls[i] = 0;
		md->zWalls[i] = 0;
		confirmed[i] = 0;
	}	
	tail = (v3){.x = 0, .y = 0, .z = 0};
	SETPOINT(confirmed, 7,7,md->depth-1);
	
	while(tail.z<md->depth){ //while(tail.z<16)
/*		DPOINT1;*/
		nextTrace(confirmed, tail);
		getNextUnvisited(confirmed, &tail);
	}
}

void MazeStart(uint8_t arg){
	if(arg&OSRESET){
		md->depth = 1;
		resetMaze();
	}
}

static inline void drawMaze(uint8_t dt);
static inline void MazeStop();
void MazeUpdate(uint8_t dt){
	if((inputDown&INPRIGHT) && (md->player.x < 7) && GETPOINTV(md->xWalls, md->player))
		++md->player.x;
	if((inputDown&INPLEFT) && md->player.x && GETPOINT(md->xWalls, md->player.x-1, md->player.y, md->player.z))
		--md->player.x;
	if((inputDown&INPDOWN) && (md->player.y < 7) && GETPOINTV(md->yWalls, md->player))
		++md->player.y;
	if((inputDown&INPUP) && md->player.y && GETPOINT(md->yWalls, md->player.x, md->player.y-1, md->player.z))
		--md->player.y;
	if((inputDown&INPA) && (md->player.z < 7) && GETPOINTV(md->zWalls, md->player))
		++md->player.z;
	if((inputDown&INPB) && md->player.z && GETPOINT(md->zWalls, md->player.x, md->player.y, md->player.z-1))
		--md->player.z;
		
	if((md->player.x == 7) && (md->player.y == 7) && (md->player.z == md->depth-1)){
		playNote(&N_hardDrop, 192);
		++md->depth;
		resetMaze();
	}
		
	drawMaze(dt);
	
	if(inputDown&INPESC)
		MazeStop();
}

static inline uint8_t getWallColor(uint8_t x, uint8_t y){
// 	int8_t dx = x - md->player.x;
// 	int8_t dy = y - md->player.y;
// 	uint8_t d = dx*dx + dy*dy;
// 	return (128-d);
	return 128;
}

static inline void drawMaze(uint8_t dt){
	++md->atimer;
	v3 c = {.z = md->player.z};
	for (c.y = 0; c.y < 8; ++c.y){
		for (c.x = 0; c.x < 8; ++c.x){
			if(!GETPOINTV(md->xWalls, c))
				canvas[c.y<<1][(c.x<<1)+1]=GAMMA(getWallColor(c.x+1, c.y));
			if(!GETPOINTV(md->yWalls, c))
				canvas[(c.y<<1)+1][c.x<<1]=GAMMA(getWallColor(c.x, c.y+1));
			canvas[(c.y<<1)+1][(c.x<<1)+1]=GAMMA(getWallColor(c.x+1, c.y+1));
 			if(GETPOINTV(md->zWalls, c))
				canvas[c.y<<1][c.x<<1] = GAMMA(48);
 			if(c.z && GETPOINT(md->zWalls, c.x,c.y,c.z-1)){
				canvas[c.y<<1][c.x<<1] = GAMMA(48);
			}
		}
	}
	uint8_t br = md->atimer << 1;
	uint8_t stdown = ~(GETPOINTV(md->zWalls, md->player)-1);
	uint8_t stup = 0;
	if(md->player.z)
		stup = ~(GETPOINT(md->zWalls, md->player.x, md->player.y, md->player.z-1)-1);
	for (uint8_t i = 0; i < 8; ++i){
		uint8_t cb = 0;
		if(br > 24){
			cb = GAMMA(br);
			br -= 24;
		}
		else{
			//br = 0;
			cb = 1;
		}
		canvas[i+8][15] = cb&stdown;
		canvas[7-i][15] = cb&stup;
		
		if(md->player.z == md->depth - i - 1){
			canvas[15][i<<1] = 255;
			canvas[15][(i<<1)+1] = 255;
		}
		else{
			if(i < md->depth)
				canvas[15][i<<1] = GAMMA(96);
			else
				canvas[15][i<<1] = 0;
			canvas[15][(i<<1)+1] = 0;
		}
// 		uint8_t lev = md->player.z == i ? 80 : 40;
// 		canvas[15][i<<1] = GAMMA(i < md->depth ? lev : 0);
// 		canvas[15][(i<<1) + 1] = canvas[15][i<<1];
	}
	
	if(md->atimer&32)
		canvas[md->player.y<<1][md->player.x<<1] = 255;
	if((md->player.z == md->depth-1) && !(md->atimer&32)){
		canvas[14][14] = 255;
	}
}

void MazeStop(){
	osSaveAndExit();
}

struct{
	uint8_t xWalls[4];
	uint8_t yWalls[4];
} *stmd;

void MazeResetTitle(){
	for (uint8_t i = 0; i < 4; ++i){
		stmd->xWalls[i] = xorshift32();
		stmd->yWalls[i] = xorshift32();
	}
}

void MazeDrawTitle(uint8_t dt){//((G[(Y)]>>(X))&1)
	for (uint8_t i = 0; i < 4; ++i){
		for (uint8_t j = 0; j < 8; ++j){
			if(GETPOINT(stmd->xWalls, j,i,0))
				canvas[(i<<1)+6][(j<<1)+1] = GAMMA(128);
			if(GETPOINT(stmd->yWalls, j,i,0))
				canvas[(i<<1)+6+1][j<<1] = GAMMA(128);
			canvas[(i<<1)+6+1][(j<<1)+1] = GAMMA(128);
		}
	}
}

GAMEIMPLEMENT(Maze, md, stmd)