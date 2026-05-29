#include <avr/io.h>
#include <avr/interrupt.h>
#include "save.h"
#include "os.h"
#include "UARTDebug.h"

uint8_t saves;

volatile uint16_t wrCount;
volatile uint16_t wrAddr;
volatile const uint8_t *wrData;

static uint8_t readByte(uint16_t p){
	while(EECR & (1<<EEPE));
	EEAR = p;
	EECR |= (1<<EERE);
	return EEDR;
}

static void writeByte(uint8_t b, uint16_t p){
	while(EECR & (1<<EEPE));
	EEAR = p;
	EEDR = b;
	EECR |= (1<<EEMPE);
	EECR |= (1<<EEPE);
}

void saveSetup(){
	uint16_t eadr = SAVEDATA;
	for (uint8_t i = 0; i < gamesCount; ++i){
		games[i]->eepMemory = eadr;
		games[i]->eepID = i;
		eadr += games[i]->memSize;
	}
	
	saves = readByte(0);
	uint8_t saveValidation = readByte(1);
	if(saveValidation != SAVEVALIDATION){
		saves = 0;
		writeByte(saves, 0);
		writeByte(SAVEVALIDATION, 1);
		while(EECR & (1<<EEPE));
		//DPOINT4;
	}
}

void saveGame(Game *game){
	while(ISSAVING);
	wrCount = game->memSize;
	wrAddr = game->eepMemory;
	wrData = *game->memory;
	
	saves |= 1<<game->eepID;
	cli();
	writeByte(saves, 0);
	sei();
	EECR |= (1<<EERIE);
}

ISR(EE_READY_vect){
	if(!wrCount){
		EECR &= ~(1<<EERIE);
		return;
	}
	--wrCount;
	if(readByte(wrAddr) != *wrData)
		writeByte(*wrData, wrAddr);
	++wrData;
	++wrAddr;
}

uint8_t restoreGame(Game *game, uint8_t* mem){
	while(ISSAVING);
	if(!(saves&(1<<game->eepID)))
		return 0;

	for (uint8_t i = 0; i < game->memSize; ++i)
		mem[i] = readByte(game->eepMemory + i);
	return 1;
}

void eraseSave(Game *game){
	while(ISSAVING);
	saves &= ~(1<<game->eepID);
	cli();
	writeByte(saves, 0);
	sei();
}
