#pragma once

#include <stdint.h>
#include <cheader.h>

CH_START

uint32_t fb_rgb2linear(uint8_t r,uint8_t g,uint8_t b);

uint8_t fb_init(char* fb,uint16_t xres,uint16_t yres,uint8_t double_buffer);

void    fb_pixel(int x,int y,uint32_t color);

void    fb_char(unsigned char c,int x,int y,uint32_t fc,uint32_t bc);

void    fb_swapbuffers();

void    fb_fill(int x,int y,int sx,int sy,uint32_t color);

CH_END
