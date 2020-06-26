#pragma once

#include <stdint.h>
#include <cheader.h>

CH_START

uint32_t fb_rgb2linear(uint8_t r,uint8_t g,uint8_t b);

uint8_t fb_init(char* fb,uint16_t xres,uint16_t yres,uint8_t double_buffer);

void    fb_pixel(int x,int y,uint32_t color,uint32_t* opt_buffer);

void    fb_line(int x,int y, int x1,int y1,uint32_t color,uint32_t* opt_buffer);

void    fb_circle(int x,int y,int rad,uint32_t color,uint8_t fill,uint32_t* opt_buffer);

void    fb_char(unsigned char c,int x,int y,uint32_t fc,uint32_t bc,uint32_t* opt_buffer);

void    fb_swapbuffers();

void    fb_rect(int x,int y,int sx,int sy,uint32_t color,uint8_t fill,uint32_t* opt_buffer);

void    fb_triangle(int x,int y,int x1,int y1,int x2,int y2,uint32_t color,uint8_t fill,uint32_t* opt_buffer);

void    fb_inject_buffer(int x,int y,int bsx,int bsy,uint32_t* buffer,uint32_t* opt_buffer); //copies rect of size (bsx,bsy) to (x,y) of draw buffer

void    fb_optbuff_size(int x,int y); //set size of opt_buffer

CH_END
