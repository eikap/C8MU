#include <stdint.h>
#include <stdio.h>
#include "display.h"
#include "mem.h"
#include "c8.h"

static const uint8_t DISPLAY_WIDTH = 64;
static const uint8_t DISPLAY_HEIGHT = 32;

bool gfx[DISPLAY_HEIGHT][DISPLAY_WIDTH];
bool drawFlag = false;

void disp_clear() {
	for (int i = 0; i < DISPLAY_HEIGHT; i++) {
		for (int j = 0; j < DISPLAY_WIDTH; j++) {
			gfx[i][j] = 0;
		}
	}
}

void draw_sprite(uint8_t x, uint8_t y, uint8_t height, uint16_t address) {
	//printf("Drawing sprite at %i %i height: %i addr: %04X\n", x, y, height, address);
	bool flipped = false;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < 8; j++) {
			if ((ram[address + i] & (0x80>>j)) != 0) {
				gfx[y + i][x + j] = !gfx[y + i][x + j];
				//printf("Flipping pixel at %i %i",x + j, y + i);
				if (gfx[y + i][x + j] == 0) {
					flipped = true;
				}
			}
		}
	}
	writeRegister(15, flipped);
	drawFlag = true;
}

char * disp_data() {
	char* data = new char[DISPLAY_WIDTH*DISPLAY_HEIGHT];
	for (int y = 0; y < DISPLAY_HEIGHT; y++) {
		for (int x = 0; x < DISPLAY_WIDTH; x++) {
			data[(31 * 64) - (y * 64) + x] = gfx[y][x]*0xFF; // Vertical Flip for OGL
		}
	}
	return data;
}
