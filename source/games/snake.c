#include "../os.h"
#include "../graphics.h"
#include "../UARTDebug.h"
#include "../input.h"

void SnakeStart(){
	
}

void SnakeStop();
void SnakeUpdate(uint8_t dt){
	if(inputUp&INPB)
		SnakeStop();
}

void SnakeStop(){
	osExitToMenu();
}

void SnakeDrawTitle(uint8_t dt){
	retrieveParam(0x12,0x02,2);
	HLINE(6,6,9,255);
	HLINE(9,6,9,255);
}


GAMEIMPLEMENT(Snake)