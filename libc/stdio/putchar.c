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
int fputc(char c, FILE* f){
	char buffer[] = {c};
	int res = fwrite(buffer,1,1,f);
	return res?res:EOF;
}
#endif

int putchar(char c){
	#ifdef __smplos_libk
		eld_putchar(c);
		return 1;
	#else
		return fputc(c,stdout);
	#endif
}
