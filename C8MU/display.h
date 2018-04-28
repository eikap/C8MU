#pragma once

extern bool gfx[32][64];
extern bool drawFlag;

void disp_clear();
void draw_sprite(uint8_t x, uint8_t y, uint8_t height, uint16_t address);
char* disp_data();