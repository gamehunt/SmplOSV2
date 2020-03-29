/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdio.h>

#ifdef __smplos_libk
	#include <kernel/dev/eld.h>
	#include <kernel/fs/vfs.h>
#else
	#include <sys/syscall.h>
#endif

#ifndef __smplos_libk
void fputc(char c, FILE* f){
	char buffer[] = {c};
	fwrite(buffer,1,1,f);
}
#endif

void putchar(char c){
	#ifdef __smplos_libk
		eld_putchar(c);
	#else
		fputc(c,stdout);
	#endif
}
