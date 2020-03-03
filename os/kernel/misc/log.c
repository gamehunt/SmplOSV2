/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/misc/log.h>
#include <kernel/io/terminal.h>
#include <kernel/dev/tty.h>

#include <stdarg.h>
#include <stdio.h>


void kinfo(const char* format,...){
	if(tty_is_enabled() && tty_get_output_stream() != TTY_OUTPUT_STREAM_STDOUT){
		tty_set_output_stream(TTY_OUTPUT_STREAM_STDOUT);
	}
	terminal_color(VGA_COLOR_WHITE);
	va_list l;
	va_start(l,format);	
	vprintf(format,l);
	va_end(l);
}

void kwarn(const char* format,...){
	if(tty_is_enabled() && tty_get_output_stream() != TTY_OUTPUT_STREAM_STDOUT){
		tty_set_output_stream(TTY_OUTPUT_STREAM_STDOUT);
	}
	terminal_color(VGA_COLOR_YELLOW);
	va_list l;
	va_start(l,format);	
	vprintf(format,l);
	va_end(l);
	terminal_color(VGA_COLOR_WHITE);
}

void kerr(const char* format,...){
	if(tty_is_enabled() && tty_get_output_stream() != TTY_OUTPUT_STREAM_STDERR){
		//kinfo("KERR UNSET\n");
		tty_set_output_stream(TTY_OUTPUT_STREAM_STDERR);
		
	}
	terminal_color(VGA_COLOR_RED);
	va_list l;
	va_start(l,format);	
	vprintf(format,l);
	va_end(l);
	terminal_color(VGA_COLOR_WHITE);
}
