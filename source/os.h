#ifndef OS_H_
#define OS_H_
#include <stdint.h>

typedef struct{
	void (*start)();
	void (*update)(uint8_t dt);
	void (*stop)();
} Game;

#define XGAMES\
			X(Settings)\
			X(Snake)\
			X(Tetris)\
			X(Arcanoid)

#define X(A) extern Game A;
XGAMES
#undef X

extern Game *games[];
#define GAMESCOUNT (sizeof(games)/sizeof(Game*))

void osSetup();

void osRun();

#endif /* OS_H_ */