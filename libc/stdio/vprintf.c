/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/


#include <stdio.h>

#ifndef __smplos_libk

int vfprintf(FILE* fd, const char* restrict format,va_list argptr){
	char buffer[4096];
	memset(buffer,0,4096);
	vsprintf(buffer,format,argptr);
	return fputs(buffer,fd);
}


#endif

int vprintf(const char* restrict format,va_list argptr){
	char buffer[4096];
	memset(buffer,0,4096);
	vsprintf(buffer,format,argptr);
	return puts(buffer);
}
