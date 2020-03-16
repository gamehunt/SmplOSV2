/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/misc/log.h>
#include <kernel/io/terminal.h>
#include <kernel/dev/tty.h>

#include <stdarg.h>
#define __smplos_libk
#include <stdio.h>


void kinfo(const char* format,...){
	set_output_stream(STDOUT);
	terminal_color(VGA_COLOR_WHITE);
	va_list l;
	va_start(l,format);	
	vprintf(format,l);
	va_end(l);
}

void kwarn(const char* format,...){
	set_output_stream(STDOUT);
	terminal_color(VGA_COLOR_YELLOW);
	va_list l;
	va_start(l,format);	
	vprintf(format,l);
	va_end(l);
	terminal_color(VGA_COLOR_WHITE);
}

void kerr(const char* format,...){
	set_output_stream(STDERR);
	terminal_color(VGA_COLOR_RED);
	va_list l;
	va_start(l,format);	
	vprintf(format,l);
	va_end(l);
	terminal_color(VGA_COLOR_WHITE);
}
