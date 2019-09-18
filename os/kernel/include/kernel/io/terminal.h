#pragma once

#include <stdint.h>
#include <kernel/video/vga.h>



void terminal_init();
void terminal_entry(int x,int y);
void terminal_putchar(char);
void terminal_color(enum vga_color c);
void terminal_back(enum vga_color c);
void terminal_scroll();
void terminal_clear();
