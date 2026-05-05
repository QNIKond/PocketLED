#ifndef OS_H_
#define OS_H_
#include <stdint.h>

typedef struct{
	void (*start)();
	void (*update)(uint8_t dt);
	void (*stop)();
	void (*drawTitle)(uint8_t dt);
} Game;

#define XGAMES\
			X(Snake)

#define X(A) extern Game A;
XGAMES
#undef X

extern Game *games[];
#define GAMESCOUNT (sizeof(games)/sizeof(Game*))

#define GAMEIMPLEMENT(G) Game G = {\
	.start = G##Start,\
	.update = G##Update,\
	.stop = G##Stop,\
	.drawTitle = G##DrawTitle,\
};

void osSetup();

void osRun();

void osExitToMenu();

#endif /* OS_H_ */