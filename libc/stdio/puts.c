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
	void fputs(const char* str,FILE* f){
		//printf("%d\n",strlen(str));
		fwrite(str,strlen(str),1,f);
	}
#endif

void puts(const char* str){
	#ifdef __smplos_libk
		eld_puts(str,strlen(str));
	#else
		fputs(str,stdout);
	#endif
}
