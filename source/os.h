#ifndef OS_H_
#define OS_H_
#include <stdint.h>

typedef struct{
	void (*start)(void* mem);
	void (*update)(uint8_t dt);
	void (*stop)();
	void (*resetTitle)(void* tmem);
	void (*drawTitle)(uint8_t dt);
} Game;

extern uint8_t __heap_start;

#define XGAMES\
			X(Snake)\
			X(Tetris)

#define X(A) extern Game A;
XGAMES
#undef X

extern Game *games[];
#define GAMESCOUNT (sizeof(games)/sizeof(Game*))

#define TITLEMINHEIGHT 6
#define TITLEMAXHEIGHT 14

#define GAMEIMPLEMENT(G) Game G = {\
	.start = G##Start,\
	.update = G##Update,\
	.stop = G##Stop,\
	.resetTitle = G##ResetTitle,\
	.drawTitle = G##DrawTitle,\
};

void osSetup();

void osRun();

void osExitToMenu();

#endif /* OS_H_ */