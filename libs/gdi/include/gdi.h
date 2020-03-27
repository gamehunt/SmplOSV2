#pragma once

#include <stdint.h>

uint32_t gdi_rgb2linear(uint8_t r,uint8_t g,uint8_t b);

uint8_t gdi_init(char* fb);

void    gdi_pixel(int x,int y,uint32_t color);
