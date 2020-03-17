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
	printf("[I] ");
	va_list l;
	va_start(l,format);	
	vprintf(format,l);
	va_end(l);
}

void kwarn(const char* format,...){
	printf("[W] ");
	va_list l;
	va_start(l,format);	
	vprintf(format,l);
	va_end(l);
}

void kerr(const char* format,...){
	printf("[E] ");
	va_list l;
	va_start(l,format);	
	vprintf(format,l);
	va_end(l);
}
