#ifndef UARTDEBUG_H_
#define UARTDEBUG_H_

#include <stdint.h>

extern uint8_t dbgInput[];
extern uint8_t dbgKeyInput;
extern uint64_t dbgFlags;

#define DPOINT1 sendMsg(0x11)
#define DPOINT2 sendMsg(0x12)
#define DPOINT3 sendMsg(0x13)
#define DPOINT4 sendMsg(0x14)
#define DPARAM1(X) sendParam(0x15, X)
#define DPARAM2(X) sendParam(0x16, X)
#define DPARAM3(X) sendParam(0x17, X)
#define DPARAM4(X) sendParam(0x18, X)
#define DRETRIEVE1(X) retrieveParam(0x11,0x02, X)
#define DRETRIEVE2(X) retrieveParam(0x12,0x02, X)
#define DRETRIEVE3(X) retrieveParam(0x13,0x02, X)
#define DRETRIEVE4(X) retrieveParam(0x14,0x02, X)

void UARTSetup();

void sendMsg(uint8_t cmd);

void sendParam(uint8_t cmd, uint8_t param);

void retrieveParam(uint8_t request, uint8_t response,  uint8_t param);

void updateParam(uint8_t request, uint8_t response, uint8_t *param);

#endif /* UARTDEBUG_H_ */