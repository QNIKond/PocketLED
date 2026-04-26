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
#define ISPINP(X,Y) (PIN##X & 1<<(PORT##X##Y))
#define TURNPON(X,Y) PORT##X |= 1<<(PORT##X##Y)
#define TURNPOFF(X,Y) PORT##X &= ~(1<<(PORT##X##Y))
#define TOGGLEP(X,Y) PORT##X ^= 1<<(PORT##X##Y)
#define BLINKP(X,Y) do {TURNPON(X,Y); TURNPOFF(X,Y);} while(0);

#define STAND

typedef int16_t fl16;
#define FLTOINT8(X) (((uint8_t*)&X)[1])

typedef int32_t fl32;
#define FLTOINT16(X) (((uint16_t*)&X)[1])

static uint32_t xs_state = 2463534242u;
static inline uint32_t xorshift32(void) {
	xs_state ^= xs_state << 13;
	xs_state ^= xs_state >> 17;
	xs_state ^= xs_state << 5;
	return xs_state;
}

static uint32_t lfsr32 = 0xA5A5A5A5u;
static inline uint32_t lfsr32_8(void){
    uint32_t lsb;
    lsb = lfsr32 & 1u; lfsr32 >>= 1; if (lsb) lfsr32 ^= 0xD0000001u;
    lsb = lfsr32 & 1u; lfsr32 >>= 1; if (lsb) lfsr32 ^= 0xD0000001u;
    lsb = lfsr32 & 1u; lfsr32 >>= 1; if (lsb) lfsr32 ^= 0xD0000001u;
    lsb = lfsr32 & 1u; lfsr32 >>= 1; if (lsb) lfsr32 ^= 0xD0000001u;
    lsb = lfsr32 & 1u; lfsr32 >>= 1; if (lsb) lfsr32 ^= 0xD0000001u;
    lsb = lfsr32 & 1u; lfsr32 >>= 1; if (lsb) lfsr32 ^= 0xD0000001u;
    lsb = lfsr32 & 1u; lfsr32 >>= 1; if (lsb) lfsr32 ^= 0xD0000001u;
    lsb = lfsr32 & 1u; lfsr32 >>= 1; if (lsb) lfsr32 ^= 0xD0000001u;

    return lfsr32;
}

static inline uint8_t lerp(uint8_t a, uint8_t b, uint8_t t){
	return (uint8_t)(a + (((uint16_t)(b - a) * (uint16_t)t + 128u) >> 8));
}

#endif /* COMMON_H_ */