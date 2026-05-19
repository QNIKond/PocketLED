/*
 * common.h
 *
 * Created: 14.04.2026 17:39:00
 *  Author: NIKond
 */ 


#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>

#define F_CPU 16000000UL
#define ISPINP(X,Y) (PIN##X & (1<<(PORT##X##Y)))
#define TURNPON(X,Y) PORT##X |= 1<<(PORT##X##Y)
#define TURNPOFF(X,Y) PORT##X &= ~(1<<(PORT##X##Y))
#define TOGGLEP(X,Y) PORT##X ^= 1<<(PORT##X##Y)
#define BLINKP(X,Y) do {TURNPON(X,Y); TURNPOFF(X,Y);} while(0);

//#define STAND

typedef struct {
	uint8_t x;
	uint8_t y;
	} v2;
#define V2ADD(U, V) do{(U).x += (V).x; (U).y += (V).y;}while(0);
typedef struct {
	uint8_t x;
	uint8_t y;
	uint8_t z;
} v3;
	
typedef union {
	uint16_t u16;
	uint8_t b[2];
}split8;

typedef int16_t fl16;
#define FLTOINT8(X) (((uint8_t*)&(X))[1])

typedef int32_t fl32;
#define FLTOINT16(X) (((uint16_t*)&(X))[1])

#define HIGH8(X) (((uint8_t*)&(X))[1])


uint32_t xorshift32(void);
#define XORRANGE(MIN, MAX) ((xorshift32()%((MAX)-(MIN))) + (MIN));


uint32_t lfsr32_8(void);

uint8_t lerp(uint8_t a, uint8_t b, uint8_t t);

#endif /* COMMON_H_ */