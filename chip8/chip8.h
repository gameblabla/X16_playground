#ifndef CHIP_8
#define CHIP_8

/*
	Based on a another CHIP8 emulator that came with no license.
	All the modifications to this CHIP8 emulator are copyright (C) 2015 Gameblabla <gameblabla@openmailbox.org>
	and are subject to the MIT license.
	See http://www.opensource.org/licenses/mit-license.php in a web browser for more information.
*/


#define dopcode memory[pc] << 8 | memory[pc + 1]
#define X       (opcode & 0x0F00) >> 8
#define Y       (opcode & 0x00F0) >> 4
#define DX      (px * SCALE)
#define DY      (py * SCALE)
#define R       (rand() % 256)
#define N       (opcode & 0x000F)
#define KK      (opcode & 0x00FF)
#define NNN     (opcode & 0x0FFF)

#define WIDTH   64
#define HEIGHT  32
/* Don't forget commas! */
#define COLOR_ON 0, 255, 0
#define COLOR_OFF 0, 0, 0

#define INST_PER_CYCLE 5

/* Key binding; don't forget ''! */
#define KEY_1 '1'
#define KEY_2 '2'
#define KEY_3 '3'
#define KEY_C '4'

#define KEY_4 'q'
#define KEY_5 'w'
#define KEY_6 'e'
#define KEY_D 'r'

#define KEY_7 'a'
#define KEY_8 's'
#define KEY_9 'd'
#define KEY_E 'f'

#define KEY_A 'z'
#define KEY_0 'x'
#define KEY_B 'c'
#define KEY_F 'v'

#define KEY_PAUSE 'p'
#define KEY_DEBUG 'b'
#define KEY_INFO 'i'
#define KEY_HALT 'h'
#define KEY_NEXT_OPCODE 'n'


uint8_t memory[4096];
unsigned char chip8_font[80] = 
{
  0xF0, 0x90, 0x90, 0x90, 0xF0, /* 0 */
  0x20, 0x60, 0x20, 0x20, 0x70, /* 1 */
  0xF0, 0x10, 0xF0, 0x80, 0xF0, /* 2 */
  0xF0, 0x10, 0xF0, 0x10, 0xF0, /* 3 */
  0x90, 0x90, 0xF0, 0x10, 0x10, /* 4 */
  0xF0, 0x80, 0xF0, 0x10, 0xF0, /* 5 */
  0xF0, 0x80, 0xF0, 0x90, 0xF0, /* 6 */
  0xF0, 0x10, 0x20, 0x40, 0x40, /* 7 */
  0xF0, 0x90, 0xF0, 0x90, 0xF0, /* 8 */
  0xF0, 0x90, 0xF0, 0x10, 0xF0, /* 9 */
  0xF0, 0x90, 0xF0, 0x90, 0x90, /* A */
  0xE0, 0x90, 0xE0, 0x90, 0xE0, /* B */
  0xF0, 0x80, 0x80, 0x80, 0xF0, /* C */
  0xE0, 0x90, 0x90, 0x90, 0xE0, /* D */
  0xF0, 0x80, 0xF0, 0x80, 0xF0, /* E */
  0xF0, 0x80, 0xF0, 0x80, 0x80  /* F */
};

uint8_t gfx[32 * 64];
uint16_t pc = 0x200;
uint16_t I = 0;
uint8_t V[16];
uint16_t opcode, inst, kk;
uint8_t px, py;
uint16_t i, ii, temp, cinst;
uint8_t sound, color, ccolor, running, paused, debug, next_opcode;
uint8_t sp;
uint16_t DT, ST;
uint16_t stack[16];
uint8_t keys[16];


void beep();

int main(int argc, char* argv[]);
void draw_pixel(int x, int y);

void Controls();
void update_screen();
void screen_init();


#endif
