#include "common.h"
static uint32_t xs_state = 2463534242u;
uint32_t xorshift32(void) {
	xs_state ^= xs_state << 13;
	xs_state ^= xs_state >> 17;
	xs_state ^= xs_state << 5;
	return xs_state;
}

static uint32_t lfsr32 = 0xA5A5A5A5u;
uint32_t lfsr32_8(void){
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

uint8_t lerp(uint8_t a, uint8_t b, uint8_t t){
	return (uint8_t)(a + (((uint16_t)(b - a) * (uint16_t)t + 128u) >> 8));
}