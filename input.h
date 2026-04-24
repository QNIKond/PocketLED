#ifndef INPUT_H_
#define INPUT_H_
#include <stdint.h>

#define INPUP (1<<0)
#define INPLEFT (1<<1)
#define INPRIGHT (1<<2)
#define INPDOWN (1<<3)
#define INPA (1<<4)
#define INPB (1<<5)
#define INPESC (1<<7)

extern uint8_t inputRaw;
extern uint8_t inputDown;
extern uint8_t inputUp;

void inputSetup();

void updateInput(uint8_t dt);

#endif /* INPUT_H_ */