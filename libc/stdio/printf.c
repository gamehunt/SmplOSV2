/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdio.h>


void printf(const char* restrict format,...){
	va_list argptr;
	va_start(argptr,format);
	vprintf(format,argptr);
	va_end(argptr);
}
