#include <cx16.h>
#include <stdio.h>
#include <stdint.h>
#include "vera.h"
#include "vload.h"

static uint8_t x, y;

#define SD_DEV      1
#define HOST_DEV    8

void __fastcall__ clear_screen(void);

#define POKE(addr,val)     (*(volatile unsigned char*) (addr) = (val))
#define POKEW(addr,val)    (*(volatile unsigned*) (addr) = (val))
#define PEEK(addr)         (*(volatile unsigned char*) (addr))
#define PEEKW(addr)        (*(volatile unsigned*) (addr))

/*
 * I noticed that writing past Y = 232 causes the pixel not to be drawn.
 * Turns out for 320x240, it's more than 65k and thus, you have to switch banks.
 * This seemingly happens even for the 160x240 color mode as well, but in this case we only lose the last 8 pixels vertically.
*/

#define my_vpoke(bank, addr, data) do { \
    *(char*)0x9f25 &= ~1; \
    *(char*)0x9f20 = ((unsigned char)((addr) & 0xFF)); \
    *(char*)0x9f21 = ((unsigned char)(((addr) >> 8))); \
    *(char*)0x9f22 = VERA_INC_0 | (bank); \
    *(char*)0x9f23 = (data); \
} while (0)

uint16_t addr;

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
		addr = (y1 << 7) + (y1 << 5); \
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
		addr = (y << 7) + (y << 5); \
		*(char*)0x9f20 = ((unsigned char)((addr + x1) & 0xFF)); \
		*(char*)0x9f21 = ((unsigned char)(((addr + x1) >> 8))); \
		*(char*)0x9f22 = VERA_INC_1 | (0); \
		*(char*)0x9f23 = (color); \
		do { \
			addr = (y << 7) + (y << 5); \
			*(char*)0x9f20 = ((unsigned char)((addr + x) & 0xFF)); \
			*(char*)0x9f21 = ((unsigned char)(((addr + x) >> 8))); \
			*(char*)0x9f23 = (color); \
		} while (--y != y1); \
	} while (0)

#define SET_PIXEL(x1, y1, c) my_vpoke(0, (y1 << 7) + (y1 << 5) + x1, c);
   
void main()
{
	uint16_t i;
	const char *palname = "pal.bin";
	const char *imgname = "test.bin";
    VERA.control = 0;
    VERA.display.video |= LAYER1_ENABLED;
    VERA.display.hscale = SCALE_4x;
    VERA.display.vscale = SCALE_4x;
    VERA.layer0.config = 0x0;  // Disable Layer 0
    VERA.layer1.config = LAYER_CONFIG_8BPP | LAYER_CONFIG_8BPP | LAYER_CONFIG_BITMAP ;         // 128x64 map, color depth 1
    VERA.layer1.mapbase = MAP_BASE_ADDR(0x0);                                       // Map base at 0x00000
    VERA.layer1.tilebase = 0;  // Tile base at 0x10000, 8x8 tiles
    videomode(VIDEOMODE_40x30);
    
    vpoke(0,0);
    // We're uploading our palette converted with https://github.com/TurboVega/image2binary
    vload(palname, HOST_DEV, 0x1fa00);
    vpoke(0,0);
    // Then we're pushing our image to the VERA chip framebuffer
    vload(imgname, HOST_DEV, 0);

	// We are setting a pixel on screen at X and Y
    SET_PIXEL(50, 50, 10);
    
    /* Slow Pixel routine */
	for (y = 0; y < 240; y++) {
		addr = (y << 7) + (y << 5);
		for (x = 0; x < 160; x++) {
			my_vpoke(0, addr + x, 5);
		}
	}
    
    // Slower Pixel routine
	for (y = 0; y < 240; y++) {
		for (x = 0; x < 160; x++) {
			my_vpoke(0, (uint16_t)y * 160 + x, 10);
		}
	}
	
	// Faster asm routine using VERA's autoincrementing
	clear_screen();
	
	// Clearing screen with routine
	for (y = 0; y < 240; y++) {
		DRAW_LINE_HORIZONTAL(0, 160, y, 10);
	}
	
	/*__asm__("stz $9F25");             // Set CTRL to 0
    __asm__("stz $9F20");             // Set primary address low byte to 0x0000
    __asm__("stz $9F21");             // Set primary address high byte to 0x0000
    __asm__("lda #%b", 0b00010000);   // Set auto-increment to 1
    __asm__("sta $9F22");
    __asm__("lda #0");                // Set the data to 0
    __asm__("ldy %v", y);            // Set y
	__asm__("ldx %v", x);
    __asm__("loop:");
    __asm__("stz $9F23");             // Write 0 to the calculated address
    __asm__("dex");                   // Decrement x
    __asm__("bne loop");              // Loop until x = 0
    __asm__("rts");*/

	//copyBankedRAMToVRAM(0, 0, 160, 160 * 2, 0);
    return;
}
