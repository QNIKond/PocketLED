#ifndef OS_H_
#define OS_H_
#include <stdint.h>

typedef struct{
	void (*start)();
	void (*stop)();
	
	void (*update)(uint8_t dt);
	void (*draw)(uint8_t dt);
} Game;

#define XGAMES\
			X(Snake)\
			X(Tetris)\
			X(Arcanoid)

#define X(A) extern Game A;
XGAMES
#undef X

extern Game *games[];

void osSetup();

void osRun();

#endif /* OS_H_ */