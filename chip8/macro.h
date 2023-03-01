#ifndef MACRO_H
#define MACRO_H

#include <cx16.h>
#include <stdio.h>
#include <stdint.h>
#include "vera.h"
#include "vload.h"

/* Global variables used for the macros */
static uint8_t x, y;
static uint16_t addr;

#define SD_DEV      1
#define HOST_DEV    8

/* Reqyures clearscreen.s and joystick.s */
void __fastcall__ clear_screen(void);

/* JOYSTICK DEFS */

#define JOY_DPAD_UP 0x8
#define JOY_DPAD_DOWN 0x4
#define JOY_DPAD_LEFT 0x2
#define JOY_DPAD_RIGHT 0x1

#define JOY_B 0x80
#define JOY_A 0x8000
#define JOY_Y 0x40
#define JOY_X 0x4000

#define JOY_LEFTSHOULDER 0x2000
#define JOY_RIGHTSHOULDER 0x1000

#define JOY_CON_START 0x10
#define JOY_CON_SELECT 0x20

extern unsigned int joystick_get(unsigned char joy_num);

#define POKE(addr,val)     (*(volatile unsigned char*) (addr) = (val))
#define POKEW(addr,val)    (*(volatile unsigned*) (addr) = (val))
#define PEEK(addr)         (*(volatile unsigned char*) (addr))
#define PEEKW(addr)        (*(volatile unsigned*) (addr))

/*
 * I noticed that writing past Y = 232 causes the pixel not to be drawn.
 * Turns out for 320x240, it's more than 65k and thus, you have to switch banks.
 * This seemingly happens even for the 160x240 color mode as well, but in this case we only lose the last 8 pixels vertically.
*/
#define _SCALE2X_MODE 1

#ifdef _SCALE4X_MODE // 160x240
#define Y_BITSHIFT(yy) (yy << 7) + (yy << 5)
#elif defined(_SCALE2X_MODE) // 320x240
#define Y_BITSHIFT(yy) (yy << 8) + (yy << 6)
#endif


#define my_vpoke(bank, addr, data) do { \
    *(char*)0x9f25 &= ~1; \
    *(char*)0x9f20 = ((unsigned char)((addr) & 0xFF)); \
    *(char*)0x9f21 = ((unsigned char)(((addr) >> 8))); \
    *(char*)0x9f22 = VERA_INC_0 | (bank); \
    *(char*)0x9f23 = (data); \
} while (0)

void copyBankedRAMToVRAM(unsigned char startMemBank, unsigned char vramBank, unsigned short vramAddr, unsigned long length, unsigned short startingImageAddr) {
    unsigned long remaining;
    unsigned short i;

    *(char*)0x9f20 = ((unsigned char)((vramAddr) & 0xFF)); \
    *(char*)0x9f21 = ((unsigned char)(((vramAddr) >> 8))); \
	*(char*)0x9f22 = VERA_INC_1 | (vramBank);

    // This crazy routine uses the kernal memory_copy function to bulk copy RAM to VRAM
    // I had to increment the bank and do it in chunks though.
    for (i=0; i<length/8192+1; i++) {
        // Set the bank to read from
        POKE(0, startMemBank+i);

        // void memory_copy(word source: r0, word target: r1, word num_bytes: r2);
        POKEW(0x2, (unsigned int)startingImageAddr);
        POKEW(0x4, 0x9F23);

        remaining = length - (8192*i);
        if (remaining < 8192) {
            POKEW(0x6, remaining);
        } else {
            POKEW(0x6, 8192);
        }
        __asm__("jsr $FEE7");
    }
}

#define DRAW_LINE_HORIZONTAL(x1, x2, y1, color) \
	do { \
		x = x2; \
		addr = Y_BITSHIFT(y1); \
		*(char*)0x9f20 = ((unsigned char)((addr + x1) & 0xFF)); \
		*(char*)0x9f21 = ((unsigned char)(((addr + x1) >> 8))); \
		*(char*)0x9f22 = VERA_INC_1 | (0); \
		*(char*)0x9f23 = (color); \
		do { \
			*(char*)0x9f20 = ((unsigned char)((addr + x) & 0xFF)); \
			*(char*)0x9f21 = ((unsigned char)(((addr + x) >> 8))); \
			*(char*)0x9f23 = (color); \
		} while (--x != x1); \
	} while (0)
	
#define DRAW_LINE_VERTICAL(y1, y2, x1, color) \
	do { \
		x = x1; \
		y = y2; \
		addr = Y_BITSHIFT(y); \
		*(char*)0x9f20 = ((unsigned char)((addr + x1) & 0xFF)); \
		*(char*)0x9f21 = ((unsigned char)(((addr + x1) >> 8))); \
		*(char*)0x9f22 = VERA_INC_1 | (0); \
		*(char*)0x9f23 = (color); \
		do { \
			addr = Y_BITSHIFT(y); \
			*(char*)0x9f20 = ((unsigned char)((addr + x) & 0xFF)); \
			*(char*)0x9f21 = ((unsigned char)(((addr + x) >> 8))); \
			*(char*)0x9f23 = (color); \
		} while (--y != y1); \
	} while (0)


#define DRAW_RECTANGLE(x1, x2, y1, y2, color) \
	do { \
		x = x2; \
		y = y2; \
		DRAW_LINE_HORIZONTAL(x1, x2, y1, color); \
		DRAW_LINE_HORIZONTAL(x1, x2, y1, color); \
		do { \
			x = x2; \
			addr = Y_BITSHIFT(y); \
			*(char*)0x9f20 = ((unsigned char)((addr + x1) & 0xFF)); \
			*(char*)0x9f21 = ((unsigned char)(((addr + x1) >> 8))); \
			*(char*)0x9f22 = VERA_INC_1 | (0); \
			*(char*)0x9f23 = (color); \
			do { \
				*(char*)0x9f20 = ((unsigned char)((addr + x) & 0xFF)); \
				*(char*)0x9f21 = ((unsigned char)(((addr + x) >> 8))); \
				*(char*)0x9f23 = (color); \
			} while (--x != x1); \
		} while (--y != y1 + 1); \
	} while (0)

#define SET_PIXEL(x1, y1, c) my_vpoke(0, Y_BITSHIFT(y1) + x1, c);

#endif
