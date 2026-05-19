#include <stdint.h>
#include "../os.h"
#include "../common.h"

// OOOOO
// O O O
//OO OOO

struct{
	uint8_t walls[8*8]; //64b
} *md;

#define SETPOINT(G, X, Y, Z) G[(Y) + ((Z)<<3)] |= 1 << (X)
#define GETPOINT(G, X, Y, Z)

static inline void getNextUnvisited(uint8_t *conf, v3 *p){
	while(conf[p->y + (p->z<<3)])
		if(--p->y == 255){
			p->y = 7;
			++p->z;
			if(p->z < 8)
				return;
		}
	p->x = 0;
	for(uint8_t row = conf[p->y + (p->z<<3)];row&1;row>>=1)
		++p->x;
}

static inline void walkRundom(uint8_t *vis, v3 *head){

}

static inline void nextTrace(uint8_t *conf, v3 *tail){
	uint8_t visited[8*8];
	v3 head = *tail;
	for(uint8_t i = 0; i < 64; ++i)
		visited[i] = 0;

	while(!GETPOINT(conf, head.x, head.y, head.z)){
		walkRundom(visited, head);
	}
}

static void genMaze(){
	uint8_t trace[8*8*2];
	uint8_t visited[8*8];
	uint8_t confirmed[8*8];
	uint8_t headID;
	
	v3 tail;

	for(uint8_t i = 0; i < 64; ++i){
		md->walls[i] = 0;
		confirmed[i] = 0;
	}	
	headID = 0;
	tail = (v3){.x = 0, .y = 7, .z = 0};
	SETPOINT(confirmed, 7,0,7);

	while(tail.z<16){
		head = tail;
		
		getNextUnvisited(&tail, confirmed);
	}
}

void MazeStart(void *mem){
	md = mem;
	rmd = mem+sizeof(*md);
	genMaze();
}

void MazeUpdate(uint8_t dt){
	
}

void MazeStop(){
	
}

void MazeResetTitle(void *mem){
	
}

void MazeDrawTitle(uint8_t dt){
	
}

GAMEIMPLEMENT(Maze)