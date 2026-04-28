#include "UARTDebug.h"
#include "common.h"
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define BAUD 9600
#define MY_UBRR (F_CPU/16/BAUD-1)

uint64_t dbgFlags;
uint8_t dbgCmds[] = {};
#define DBGINCOUNT sizeof(dbgCmds)/sizeof(uint8_t)
uint8_t dbgInput[DBGINCOUNT];
uint8_t dbgKeyInput;

void UARTSetup(){
	UBRR0H = (unsigned char)(MY_UBRR >> 8);
	UBRR0L = (unsigned char)MY_UBRR;
	
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void sendMsg(uint8_t cmd){
	while (!(UCSR0A & (1 << UDRE0))); 
	UDR0 = cmd;
}

void sendParam(uint8_t cmd, uint8_t param){
	sendMsg(cmd);
	sendMsg(param);
}

void retrieveParam(uint8_t request, uint8_t response, uint8_t param){
	if(dbgFlags&(1UL<<request)){
		dbgFlags &= ~(1UL<<request);
		sendParam(response, param);
	}
}

void updateParam(uint8_t request, uint8_t response, uint8_t *param){
	if(dbgFlags&(1UL<<request)){
		dbgFlags &= ~(1UL<<request);
		for (uint8_t i = 0; i < DBGINCOUNT; ++i){
			if(dbgCmds[i] == request){
				*param = dbgInput[i];
				break;
			}
		}
		sendParam(response, *param);
	}
}

ISR(USART_RX_vect) {
	static uint8_t c = 0;
	static uint8_t cmd = 0;
	
	uint8_t data = UDR0;
	if(c){
		c = 0;
		dbgFlags |= 1UL<<cmd;
		for (uint8_t i = 0; i < DBGINCOUNT; ++i){
			if(dbgCmds[i] == cmd){
				dbgInput[i] = data;
				break;
			}
		}
	}
	else{
		if((data>>4)==0x0F){
			uint8_t inp = (data&0x0F)>>1;
			dbgKeyInput = (dbgKeyInput&(~(1<<inp))) | (((~data)&1)<<inp);
		}
		else{
			++c;
			cmd = data;
		}
	}
}
