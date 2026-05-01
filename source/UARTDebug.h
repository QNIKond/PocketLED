#ifndef UARTDEBUG_H_
#define UARTDEBUG_H_

#include <stdint.h>

extern uint8_t dbgInput[];
extern uint8_t dbgKeyInput;
extern uint64_t dbgFlags;

void UARTSetup();

void sendMsg(uint8_t cmd);

void sendParam(uint8_t cmd, uint8_t param);

void retrieveParam(uint8_t request, uint8_t response,  uint8_t param);

void updateParam(uint8_t request, uint8_t response, uint8_t *param);

#endif /* UARTDEBUG_H_ */