/*
	Based on a another CHIP8 emulator that came with no license.
	All the modifications to this CHIP8 emulator are copyright (C) 2015 Gameblabla <gameblabla@openmailbox.org>
	and are subject to the MIT license.
	See http://www.opensource.org/licenses/mit-license.php in a web browser for more information.
*/


#include <stdio.h>
#include <stdint.h>qss
#include <string.h>
#include <time.h>
#include <conio.h>

#include "chip8.h"

uint8_t j;

extern void waitvsync();

#define SCALEX 2
#define SCALEY 3

#ifdef SDL
#define SET_PIXEL(x,y,c) *((uint8_t*)screen->pixels + x + y * (WIDTH*SCALE)) = c;
#include "SDL/SDL.h"
#include "SDL/SDL_keysym.h"
SDL_Surface* screen;
SDL_Event event;
uint8_t x,y;
#define UPDATE_SCREEN() SDL_Flip(screen);
#else
#include <cbm.h>
#include <cx16.h>
#include "macro.h"
#include "load.h"
#include "ym2151.h"
#include "soundfx.h"
#define UPDATE_SCREEN()
// found in common.s
extern uint32_t seed;
#pragma zpsym("seed")
extern uint8_t fastcall galois16o(void);

// Implementing RAND with a PRNG by Brad Smith
uint8_t rand() 
{
	return galois16o();
}

struct soundFx_t sampleFx = {
    0,0,
    0x10,   // Channel 5
    {
        YM_OP_CTRL+YM_CH_5, YM_RL_ENABLE|YM_CON_ALL_PL,
        YM_KS_AR+YM_CH_5, 0xf1,
        YM_AMS_EN_D1R+YM_CH_5, 0xa0,
        YM_D1L_RR+YM_CH_5, 0xff,
        YM_KC+YM_CH_5, YM_KC_OCT5|0xe,
        YM_KEY_ON, YM_CH_5|YM_SN_ALL,
        FX_DELAY_REG,1,
        YM_KC+YM_CH_5, YM_KC_OCT6|0x1,
        YM_KEY_ON, YM_CH_5|YM_SN_ALL,
        FX_DELAY_REG,1,
        YM_KC+YM_CH_5, YM_KC_OCT6|0x4,
        YM_KEY_ON, YM_CH_5|YM_SN_ALL,
        FX_DELAY_REG,1,
        YM_KC+YM_CH_5, YM_KC_OCT6|0x5,
        YM_KEY_ON, YM_CH_5|YM_SN_ALL,
        FX_DELAY_REG,1,
        YM_KC+YM_CH_5, YM_KC_OCT6|0x8,
        YM_KEY_ON, YM_CH_5|YM_SN_ALL,
        FX_DELAY_REG,1,
        YM_KC+YM_CH_5, YM_KC_OCT6|0xa,
        YM_KEY_ON, YM_CH_5|YM_SN_ALL,
        FX_DELAY_REG,1,
        YM_KC+YM_CH_5, YM_KC_OCT6|0xd,
        YM_KEY_ON, YM_CH_5|YM_SN_ALL,
        FX_DELAY_REG,1,
        YM_KC+YM_CH_5, YM_KC_OCT6|0xe,
        YM_KEY_ON, YM_CH_5|YM_SN_ALL,
        FX_DELAY_REG,1,
        YM_KC+YM_CH_5, YM_KC_OCT7|0x1,
        YM_KEY_ON, YM_CH_5|YM_SN_ALL,
        FX_DELAY_REG,1,
        YM_KC+YM_CH_5, YM_KC_OCT7|0x4,
        YM_KEY_ON, YM_CH_5|YM_SN_ALL,
        FX_DELAY_REG,1,
        YM_KC+YM_CH_5, YM_KC_OCT7|0x5,
        YM_KEY_ON, YM_CH_5|YM_SN_ALL,
        FX_DELAY_REG,1,
        YM_KC+YM_CH_5, YM_KC_OCT7|0x8,
        YM_KEY_ON, YM_CH_5|YM_SN_ALL,
        FX_DELAY_REG,1,
        YM_KC+YM_CH_5, YM_KC_OCT7|0xa,
        YM_KEY_ON, YM_CH_5|YM_SN_ALL,
        FX_DELAY_REG,1,
        YM_KC+YM_CH_5, YM_KC_OCT7|0xd,
        YM_KEY_ON, YM_CH_5|YM_SN_ALL,
        FX_DELAY_REG,1,
        YM_KC+YM_CH_5, YM_KC_OCT7|0xe,
        YM_KEY_ON, YM_CH_5|YM_SN_ALL,
        FX_DELAY_REG,1,
        YM_KEY_ON, YM_CH_5,
        FX_DONE_REG, 0,
    }
};

#endif

#define draw_double_pixel(x,y,color) \
    for (i = 0; i < SCALEX; i++) { \
        for (j = 0; j < SCALEY; j++) { \
			SET_PIXEL((x*SCALEX + i) + 16, (y*SCALEY + j) + 8, color); \
        } \
    } \

char* keyname;
char* key;
char chr;


void beep() 
{
    if (sound)
    {
		playFx(&sampleFx);
    }
    else
    {
		//stop_beep();
	}
}

uint8_t maxrom[4096 - 0x200];
uint32_t joy[2];

/* This is done to work around an issue with the default functions chopping off the first 2 bytes from files.
 * setlfs needs to be set to 2 for that NOT to happen. */
uint16_t myload_file(const char *fileName, uint16_t addr)
{
	uint16_t size;
    cbm_k_setlfs(1,HOST_DEV,2);
    cbm_k_setnam(fileName);
    size = cbm_k_load(0,addr) - addr;
    // If file loading failed then let's try from SD instead
    if (size == 0)
    {
		cbm_k_setlfs(1,SD_DEV,2);
		cbm_k_setnam(fileName);
		size = cbm_k_load(0,addr) - addr;
	}
    return size;
}

void Controls_chip8();

#define KEY_CHIP8_LEFT 0
#define KEY_CHIP8_RIGHT 1
#define KEY_CHIP8_UP 2
#define KEY_CHIP8_DOWN 3
#define KEY_CHIP8_A 4
#define KEY_CHIP8_B 5
#define KEY_CHIP8_X 5
#define KEY_CHIP8_Y 6
uint8_t KEY_DEFINE[15];

char str[18];
 
int main(int argc, char** argv) 
{
    uint16_t result = 0;
    uint16_t size = 0;
    
    puts("CHIP-8 Emulator. Port by Gameblabla. Build 01");
	puts("Enter ROM filename (Try lowercase if this doesn't work) :");
	gets(str);
    printf("ROM is %s\n", str);
    
	size = myload_file(str, maxrom);
	if (size == 0)
	{
		puts("Error : Incorrect filename or failed to load !");
		return 1;
	}
	memcpy(memory + 0x200, maxrom, size);
	
	// Set controls according to game
	memset(KEY_DEFINE, 0, sizeof(KEY_DEFINE));
	
	cinst = INST_PER_CYCLE;
	
	switch(size)
	{
		// Game is breakout
		case 280:
			KEY_DEFINE[KEY_CHIP8_LEFT] = 4;
			KEY_DEFINE[KEY_CHIP8_RIGHT] = 6;
		break;
		// Game is Tetris
		// The falling down key doesn't work OR it's already too fast
		// TODO FIX
		case 494:
			KEY_DEFINE[KEY_CHIP8_LEFT] = 5;
			KEY_DEFINE[KEY_CHIP8_RIGHT] = 6;
			
			KEY_DEFINE[KEY_CHIP8_DOWN] = 3;
			
			KEY_DEFINE[KEY_CHIP8_A] = 4; // Rotate
			KEY_DEFINE[KEY_CHIP8_B] = 2; // Falling Down
		break;
		// Game is Puckman
		case 2356:
		default:
			KEY_DEFINE[KEY_CHIP8_LEFT] = 7; //A
			KEY_DEFINE[KEY_CHIP8_RIGHT] = 8; //S
			
			KEY_DEFINE[KEY_CHIP8_UP] = 3; // 3
			KEY_DEFINE[KEY_CHIP8_DOWN] = 6; //E

			KEY_DEFINE[KEY_CHIP8_A] = 5;
			KEY_DEFINE[KEY_CHIP8_B] = 7;
		break;
	}

    /* Load hex font */
    for (temp=0; temp < 80; temp++) 
    {
		memory[temp] = chip8_font[temp];
	}
 
#ifdef SDL
	SDL_Init(SDL_INIT_VIDEO);
	screen = SDL_SetVideoMode(WIDTH*SCALE, HEIGHT*SCALE, 8, SDL_SWSURFACE);
#else
    VERA.control = 0;
    VERA.display.video |= LAYER1_ENABLED;
    VERA.display.hscale = SCALE_4x;
    VERA.display.vscale = SCALE_4x;
    VERA.layer0.config = 0x0;  // Disable Layer 0
    VERA.layer1.config = LAYER_CONFIG_8BPP | LAYER_CONFIG_BITMAP;         // 128x64 map, color depth 1
    VERA.layer1.mapbase = MAP_BASE_ADDR(0x0);                                       // Map base at 0x00000
    VERA.layer1.tilebase = 0;  // Tile base at 0x10000, 8x8 tiles
    videomode(VIDEOMODE_40x30);
#endif
    
    /* Setup some variables */
    running = 1;
    
	#ifndef SDL
    clear_screen();
	#endif
	
	seed = 1;
    
    do
    {
		Controls_chip8();
		
		if (paused && next_opcode) cinst = 1;
        for (ii = 0; ii < cinst; ii++) {
        if (paused) cinst = 0;
        /* It looks better without identation */
        if (pc >= 4096) 
        {
            break;
        }
        opcode = dopcode;
        inst   = opcode & 0xF000;
        pc += 2;
        switch(inst) 
        {
            case 0x0000:
                switch (KK) 
                {
                    case 0x00E0:
						#ifdef SDL
                        for (y=0; y<32; y++) 
                        {
                            for (x=0; x<64; x++) 
                            {
                                gfx[(y*64)+x] = 0;
                            }
                        }
                        #else
                        bzero(gfx, 32 * 64);
                        #endif
						UPDATE_SCREEN()
                        break;
                    case 0x00EE:
                        pc = stack[sp--];
                        break;
                    default:
                        /* 0NNN - ignored */
                        break;
                }
                break;
            case 0x1000:
                pc = NNN;
                break;
            case 0x2000:
                stack[++sp] = pc;
                pc = NNN;
                break;
            case 0x3000:
                if (V[X] == KK)
                    pc += 2;
                break;
            case 0x4000:
                if (V[X] != KK)
                    pc += 2;
                break;
            case 0x5000:
                if (V[X] == V[Y])
                    pc += 2;
                break;
            case 0x6000:
                V[X] = KK;
                break;
            case 0x7000:
                V[X] += KK;
                break;
            case 0x8000:
                switch (opcode & 0x000F) {
                    case 0x0000:
                        V[X] = V[Y];
                        break;
                    case 0x0001:
                        V[X] |= V[Y];
                        break;
                    case 0x0002:
                        V[X] &= V[Y];
                        break;
                    case 0x0003:
                        V[X] ^= V[Y];
                        break;
                    case 0x0004:
                        temp = V[X] + V[Y];
                        V[0xF] = (temp > 0xFF);
                        V[X] = temp;
                        break;
                    case 0x0005:
                        temp = V[X] - V[Y];
                        V[0xF] = (V[X] > V[Y]);
                        V[X] = temp;
                        break;
                    case 0x0006:
                        V[0xF] = V[X] & 1;
                        V[X] >>= 1;
                        break;
                    case 0x0007:
                        temp = V[Y] - V[X];
                        V[0xF] = (V[Y] > V[X]);
                        V[X] = temp;
                        break;
                    case 0x000E:
                        V[0xF] = (V[X] & 128) >> 7;
                        V[X] <<= 1;
                        break;
                    default:
                        break;
                }
                break;
            case 0x9000:
                if (V[X] != V[Y])
                    pc += 2;
                break;
            case 0xA000:
                I = NNN;
                break;
            case 0xBA00:
                pc = NNN + V[0x0];
                break;
            case 0xC000:
				/*srand(0);*/
                V[X] = R & KK;
                break;
            case 0xD000:
                /* Das drawing! */
                V[0xF] = 0;
                px = py = 0;
                for (y=0; y < N; y++) 
                {
                    py = ((V[Y] + y) % 32);
                    for (x=0; x < 8; x++) 
                    {
                        px = ((V[X] + x) % 64);
                        color = (memory[I + y] >> (7 - x)) & 1;
                        ccolor = gfx[(py*64)+px];
                        if (color && ccolor) V[0xF] = 1;
                        color ^= ccolor;
                        gfx[(py*64)+px] = color;
                        #ifdef SDL
                        int c;
						if (color)
						{
							c = SDL_MapRGB(screen->format, 255, 0, 0);
						}
						else
						{
							c = SDL_MapRGB(screen->format, 0, 0, 0);
						}
						draw_double_pixel(px, py, c);
						#else
						draw_double_pixel(px, py, color);
						//SET_PIXEL(px*2, py*2, color);
						//fill_rectangle(x, y+60, SCALE, 0, (color ? 255 : 0), 0);
						//SET_PIXEL(DX, DY, color);
						#endif
                    }
                }
                UPDATE_SCREEN()
                break;
            case 0xE000:
                switch (KK) {
                    case 0x009E:
                        if (keys[V[X]] == 1)
                            pc += 2;
                        break;
                    case 0x00A1:
                        if (keys[V[X]] == 0)
                            pc += 2;
                        break;
                }
                break;
            case 0xF000:
                switch (KK) {
                    case 0x0007:
                        V[X] = DT;
                        break;
                    case 0x000A:
                        temp = -1;
                        for (i=0; i<16; i++) {
                            if (keys[i]) {
                                temp = i;
                                break;
                            }
                        }
                        if (temp != -1)
                            V[X] = temp;
                        else
                            pc -= 2;
                        break;
                    case 0x0015:
                        DT = V[X];
                        break;
                    case 0x0018:
                        ST = V[X];
                        break;
                    case 0x001E:
                        I += V[X];
                        break;
                    case 0x0029:
                        I = V[X] * 5;
                        break;
                    case 0x0033:
                        memory[I] = V[X] / 100;
                        memory[I + 1] = (V[X] / 10) % 10;
                        memory[I + 2] = V[X] % 10;
                        break;
                    case 0x0055:
                        for (i=0; i<=X; i++)
                            memory[I + i] = V[i];
                        break;
                    case 0x0065:
                        for (i=0; i<=X; i++)
                            V[i] = memory[I + i];
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
			if (DT > 0) 
			{
				DT--;
			}
			if (ST > 0) 
			{
				ST--;
				if (!ST) beep();
			}
        }
    } 
    while (running);

    // Resets hardware (apparently)
     POKE(0, 0);
    __asm__("jmp $FFFC");
    
    return 0;
}



void Controls_chip8()
{

#ifndef SDL
	memset(keys, 0, sizeof(keys));
	for(i=0;i<2;i++)
	{
		joy[i] = joystick_get(i);
		if (!(joy[i] & JOY_DPAD_LEFT))
		{
			keys[KEY_DEFINE[KEY_CHIP8_LEFT]] = 1;
		}
		else if (!(joy[i] & JOY_DPAD_RIGHT))
		{
			keys[KEY_DEFINE[KEY_CHIP8_RIGHT]] = 1;
		}

		if (!(joy[i] & JOY_DPAD_UP))
		{
			keys[KEY_DEFINE[KEY_CHIP8_UP]] = 1;
		}
		else if (!(joy[i] & JOY_DPAD_DOWN))
		{
			keys[KEY_DEFINE[KEY_CHIP8_DOWN]] = 1;
		}


		if (!(joy[i] & JOY_A))
		{
			keys[KEY_DEFINE[KEY_CHIP8_A]] = 1;
		}
		if (!(joy[i] & JOY_B))
		{
			keys[KEY_DEFINE[KEY_CHIP8_B]] = 1;
		}
		
		if (!(joy[i] & JOY_X))
		{

		}
		if (!(joy[i] & JOY_Y))
		{

		}
		
		if (!(joy[i] & JOY_CON_SELECT))
		{
			running = 0;
		}
	}
#else

        if (SDL_PollEvent(&event)) 
        {
            switch (event.type) 
            {
                case (SDL_QUIT):
                    running = 0;
                    break;
                case (SDL_KEYDOWN):
                    key = SDL_GetKeyName(event.key.keysym.sym);
                    if (strlen(key) > 1) break;
                    chr = key[0];
                    switch(chr) {
                        case KEY_1:
                            keys[0] = 1;
                            break;
                        case KEY_2:
                            keys[1] = 1;
                            break;
                        case KEY_3:
                            keys[2] = 1;
                            break;
                        case KEY_C:
                            keys[3] = 1;
                            break;
                            
                        case KEY_4:
                            keys[4] = 1;
                            break;
                        case KEY_5:
                            keys[5] = 1;
                            break;
                        case KEY_6:
                            keys[6] = 1;
                            break;
                        case KEY_D:
                            keys[7] = 1;
                            break;
                            
                        case KEY_7:
                            keys[8] = 1;
                            break;
                        case KEY_8:
                            keys[9] = 1;
                            break;
                        case KEY_9:
                            keys[10] = 1;
                            break;
                        case KEY_E:
                            keys[11] = 1;
                            break;
                            
                        case KEY_A:
                            keys[12] = 1;
                            break;
                        case KEY_0:
                            keys[13] = 1;
                            break;
                        case KEY_B:
                            keys[14] = 1;
                            break;
                        case KEY_F:
                            keys[15] = 1;
                            break;
                        
                        case KEY_NEXT_OPCODE:
                            next_opcode = 1;
                    }
                    break;
                case (SDL_KEYUP):
                    key = SDL_GetKeyName(event.key.keysym.sym);
                    if (strlen(key) > 1) break;
                    chr = key[0];
                    switch(chr) {
                        case KEY_1:
                            keys[0] = 0;
                            break;
                        case KEY_2:
                            keys[1] = 0;
                            break;
                        case KEY_3:
                            keys[2] = 0;
                            break;
                        case KEY_C:
                            keys[3] = 0;
                            break;
                            
                        case KEY_4:
                            keys[4] = 0;
                            break;
                        case KEY_5:
                            keys[5] = 0;
                            break;
                        case KEY_6:
                            keys[6] = 0;
                            break;
                        case KEY_D:
                            keys[7] = 0;
                            break;
                            
                        case KEY_7:
                            keys[8] = 0;
                            break;
                        case KEY_8:
                            keys[9] = 0;
                            break;
                        case KEY_9:
                            keys[10] = 0;
                            break;
                        case KEY_E:
                            keys[11] = 0;
                            break;
                            
                        case KEY_A:
                            keys[12] = 0;
                            break;
                        case KEY_0:
                            keys[13] = 0;
                            break;
                        case KEY_B:
                            keys[14] = 0;
                            break;
                        case KEY_F:
                            keys[15] = 0;
                            break;
                        case KEY_PAUSE:
                            paused = !paused;
                            if (paused) {
                                SDL_WM_SetCaption(
                                    "CHIP8 emulator - paused", 0);
                                cinst = 0;
                            }
                            else {
                                SDL_WM_SetCaption(
                                    "CHIP8 emulator - running", 0);
                                cinst = INST_PER_CYCLE;
                            }
                            break;
                        case KEY_NEXT_OPCODE:
                            if (paused) 
                            {
                                next_opcode = 0;
                            }
                            break;
                        case KEY_INFO:
                            for (temp=0; temp<16; temp++) {
                                printf("V%-2d ", temp);
                            }
                            printf("\n");
                            for (temp=0; temp<16; temp++) {
                                printf("%-3d ", V[temp]);
                            }
                            printf("\n");
                            printf("I: %d\n", I);
                            printf("pc: %d\n", pc);
                            printf("stack pointer: %d\n", sp);
                            printf("Stimer: %d\n", ST);
                            printf("Dtimer: %d\n", DT);
                            break;
                        case KEY_HALT:
                            running = 0;
                            break;
                    }
                    break;
            }
        }
     
#endif
       
}
