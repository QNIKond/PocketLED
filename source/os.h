#ifndef OS_H_
#define OS_H_
#include <stdint.h>

typedef struct{
	void (*start)(uint8_t arg);
	void (*update)(uint8_t dt);
	void (*stop)();
	void (*resetTitle)();
	void (*drawTitle)(uint8_t dt);
	void **memory;
	uint8_t memSize;
	void **tcmemory;
	uint16_t eepMemory;
	uint8_t eepID;
} Game;

extern uint8_t __heap_start;

#define XGAMES\
			X(Snake)\
			X(Tetris)\
			X(Maze)

#define X(A) extern Game A;
XGAMES
#undef X

extern Game *games[];
extern uint8_t gamesCount;

#define TITLEMINHEIGHT 6
#define TITLEMAXHEIGHT 14
#define OSRESET 1

#define GAMEIMPLEMENT(G, M, TCM) Game G = {\
	.start = G##Start,\
	.update = G##Update,\
	.stop = G##Stop,\
	.resetTitle = G##ResetTitle,\
	.drawTitle = G##DrawTitle,\
	.memory = (void**)&M,\
	.memSize = sizeof(*M),\
	.tcmemory = (void**)&TCM\
};

void osSetup();

void osRun();

void osSaveAndExit();

#endif /* OS_H_ */