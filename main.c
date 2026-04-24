/*
 * MatrixTest3.c
 *
 * Created: 04.03.2026 9:06:59
 * Author : NIKond
 */ 

#include "common.h"
#include <avr/io.h>
#include <stdint.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <string.h>
#include "LEDMatrix.h"
#include "input.h"
#include "UARTDebug.h"
#include "sound.h"

#define FRQ(X) ((F_CPU / (2UL * 8 * X)) - 1)
void switchBeep(uint16_t freq){
	if(freq == 0){
		TCCR1B &= ~_BV(CS11);
// 		TCCR1A &= ~_BV(COM1B0);
// 		PORTB &= ~_BV(PORTB2);
// 		TCCR1A |= _BV(COM1B0);
		TCCR1A &= ~_BV(COM1B0);
		TCCR1A |= _BV(COM1B1);
		TCCR1C |= _BV(FOC1B);
		TCCR1A &= ~_BV(COM1B1);
		TCCR1A |= _BV(COM1B0);
	}
	else{
		OCR1A = freq;
		TCCR1B |= _BV(CS11);
		

	}
}
Note asdf;
void setup(){
	cli();
	inputSetup();
	LEDMatrixSetup();
	UARTSetup();
	soundSetup();
	sei();
}

uint8_t ipp;
void inpp(){
	TURNPON(D,6);
	ipp = 255;
	for (int i = 0; i < 8; ++i){
		ipp ^= ISPINP(D,7)>>i;
		BLINKP(D,5);
	}

	// 	for (int i = 0; i < 8; ++i){
	// 		if(ISPINP(D,7)){
	// 			inputRaw[i];
	// 		}
	// 	 	BLINKP(D,5);
	//  	}

	TURNPOFF(D,6);
}

void animate(uint8_t screen[16][16])
{
	enum { W = 16, H = 16, N_BLOBS = 6 };

	// persistent state
	static uint8_t buf[H][W];           // internal brightness buffer (0-255)
	static int xpos[N_BLOBS];           // fixed-point positions (24.8 fixed point)
	static int ypos[N_BLOBS];
	static int velx[N_BLOBS];           // fixed-point velocities (24.8)
	static int vely[N_BLOBS];
	static uint8_t strength[N_BLOBS];   // strength (0-255)
	static uint32_t rng;                // simple LCG RNG
	static int initialized = 0;
	static int tick = 0;

	if (!initialized) {
		// deterministic seed (change if you want different runs)
		rng = 123456789u;
		for (int k = 0; k < N_BLOBS; ++k) {
			// LCG: rng = rng * 1664525 + 1013904223 (32-bit)
			rng = rng * 1664525u + 1013904223u;
			xpos[k] = (int)((rng >> 8) & 0xFF) << 8; // pixel * 256
			rng = rng * 1664525u + 1013904223u;
			ypos[k] = (int)((rng >> 8) & 0xFF) << 8;

			rng = rng * 1664525u + 1013904223u;
			// small velocities in fixed point: range -96..+96 (~ -0.375 .. +0.375 px/frame)
			velx[k] = (int)((int32_t)(rng & 0x1FF) - 0x100) / 2;
			rng = rng * 1664525u + 1013904223u;
			vely[k] = (int)((int32_t)(rng & 0x1FF) - 0x100) / 2;

			// keep velocities not zero-ish
			if (velx[k] == 0) velx[k] = ((k & 1) ? 32 : -32);
			if (vely[k] == 0) vely[k] = ((k & 2) ? 48 : -48);

			// strengths low to keep many dark pixels; choose 12..48
			rng = rng * 1664525u + 1013904223u;
			strength[k] = 12 + (uint8_t)((rng >> 8) & 31);
		}
		// start from near-dark
		memset(buf, 0, sizeof(buf));
		initialized = 1;
	}

	++tick;

	// 1) Move blobs (fixed-point). Bounce off edges.
	for (int k = 0; k < N_BLOBS; ++k) {
		xpos[k] += velx[k];
		ypos[k] += vely[k];

		// bounds in pixels: 0 .. (W-1) with fixed-point rounding
		const int xmin = 0 << 8;
		const int xmax = (W - 1) << 8;
		const int ymin = 0 << 8;
		const int ymax = (H - 1) << 8;

		if (xpos[k] < xmin) { xpos[k] = xmin; velx[k] = -velx[k]; }
		else if (xpos[k] > xmax) { xpos[k] = xmax; velx[k] = -velx[k]; }

		if (ypos[k] < ymin) { ypos[k] = ymin; vely[k] = -vely[k]; }
		else if (ypos[k] > ymax) { ypos[k] = ymax; vely[k] = -vely[k]; }

		// occasional tiny random jitter or strength wobble for life
		if ((tick + k * 17) % 180 == 0) {
			rng = rng * 1664525u + 1013904223u;
			int dx = (int)((rng >> 8) & 7) - 3; // -3..+3
			rng = rng * 1664525u + 1013904223u;
			int dy = (int)((rng >> 8) & 7) - 3;
			velx[k] += dx * 8;
			vely[k] += dy * 8;

			// wobble strength slightly but keep low
			rng = rng * 1664525u + 1013904223u;
			int sdelta = (int)((rng >> 8) & 7) - 3;
			int s = (int)strength[k] + sdelta;
			if (s < 8) s = 8;
			if (s > 64) s = 64;
			strength[k] = (uint8_t)s;
		}
	}

	// 2) Decay buffer: gentle exponential-like decay (integer only)
	for (int y = 0; y < H; ++y) {
		for (int x = 0; x < W; ++x) {
			uint8_t v = buf[y][x];
			// decay = v/16 + 1  -> faster when bright, leaves low residuals
			uint8_t dec = (v >> 4) + 1u;
			if (dec >= v) buf[y][x] = 0;
			else buf[y][x] = v - dec;
		}
	}

	// 3) Inject blobs into buffer with small radial falloff (Manhattan distance)
	//    radius small (4) and low strengths to keep image dark overall.
	const int R = 4;
	for (int k = 0; k < N_BLOBS; ++k) {
		int cx = xpos[k] >> 8;
		int cy = ypos[k] >> 8;
		int s = strength[k];

		int x0 = cx - R; if (x0 < 0) x0 = 0;
		int x1 = cx + R; if (x1 > (W - 1)) x1 = W - 1;
		int y0 = cy - R; if (y0 < 0) y0 = 0;
		int y1 = cy + R; if (y1 > (H - 1)) y1 = H - 1;

		for (int y = y0; y <= y1; ++y) {
			for (int x = x0; x <= x1; ++x) {
				int dx = cx - x; if (dx < 0) dx = -dx;
				int dy = cy - y; if (dy < 0) dy = -dy;
				int dist = dx + dy; // Manhattan distance - cheap and smooth enough
				if (dist <= R) {
					// add = strength * (R - dist) / R  -> integer division
					int add = (s * (R - dist)) / R;
					int nv = (int)buf[y][x] + add;
					buf[y][x] = (nv > 255) ? 255 : (uint8_t)nv;
				}
			}
		}
	}

	// 4) One-pass diffusion / blur to smooth transitions (keeps everything soft)
	//    tmp = (self*2 + up + down + left + right) / 6  -> conserves energy roughly
	uint8_t tmp[H][W];
	for (int y = 0; y < H; ++y) {
		for (int x = 0; x < W; ++x) {
			int sum = (int)buf[y][x] * 2;
			sum += (x > 0) ? buf[y][x - 1] : buf[y][x];
			sum += (x < W - 1) ? buf[y][x + 1] : buf[y][x];
			sum += (y > 0) ? buf[y - 1][x] : buf[y][x];
			sum += (y < H - 1) ? buf[y + 1][x] : buf[y][x];
			tmp[y][x] = (uint8_t)(sum / 6);
		}
	}

	// copy tmp back to buf for next frame
	memcpy(buf, tmp, sizeof(buf));

	// 5) Map to screen with gamma correction. Keep values low-to-mid for "mostly dark"
	//    Optionally scale down overall brightness slightly to emphasize darkness.
	//    scale factor: divide by 1..4. We'll divide by 1 (no change) but could use >>1 to be darker.
	for (int y = 0; y < H; ++y) {
		for (int x = 0; x < W; ++x) {
			uint8_t v = buf[y][x];
			// optional global darkening: uncomment to make darker
			// v = v >> 1;
			screen[y][x] = GAMMA(v);
		}
	}
}

void onePixelTest(){
	static uint8_t c = 0;
	static uint8_t r = 0;
	canvas[r][c] = 255;
	c = (c+1);
	if(c >= 16)
	{
		c = 0;
		r = (r+1)%16;
	}
}
void gradient(){
	for(uint8_t i = 0; i < 16; ++i)
		for(uint8_t j = 0; j < 16; ++j)
			canvas[i][j] = (i<<4) + j;
}
void gradientWGamma(){
	for(uint8_t i = 0; i < 16; ++i)
	for(uint8_t j = 0; j < 16; ++j)
	canvas[15-i][15-j] = GAMMA((i<<4) + j);
}

#define MSPT 20
int main(void){
	setup();
	
	uint8_t c = 0;
	uint8_t x = 8;
	uint8_t y = 8;
	
	uint64_t prevTime = mtime;
    while (1){
		if(inputDown&INPLEFT)
			x = x == 0 ? 0 : x-1;
		if(inputDown&INPRIGHT)
			x = x == 15 ? 15 : x+1;
		if(inputDown&INPUP)
			y = y == 0 ? 0 : y-1;
		if(inputDown&INPDOWN)
			y = y == 15 ? 15 : y+1;
			
		if(inputUp&INPA)
			sendParam(0x03, x);
		if(inputUp&INPB)
			sendParam(0x04, y);
			
		if(inputRaw&INPESC){
			x = 8;
			y = 8;
		}
		
		retrieveParam(0x11, 0x15, tone.grain);
		updateParam(0x15, 0x02, &c);
			
		gradientWGamma();
		canvas[y][x] = 0;
		flushScreenAndWait();
		updateInput(mtime - prevTime);
		prevTime = mtime;
		_delay_ms(MSPT);
    }
}

