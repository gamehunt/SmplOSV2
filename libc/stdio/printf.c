/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdio.h>

#ifndef __smplos_libk
int fprintf(FILE* fd,const char* restrict format,...){
	va_list argptr;
	va_start(argptr,format);
	vfprintf(fd,format,argptr);
	va_end(argptr);
	return 0;
}
#endif

void printf(const char* restrict format,...){

	va_list argptr;
	va_start(argptr,format);
	#ifdef __smplos_libk
		vprintf(format,argptr);
	#else
		vfprintf(stdout,format,argptr);
	#endif
	va_end(argptr);

}
