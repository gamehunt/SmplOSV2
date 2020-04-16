/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdio.h>

int sprintf(char* buf,const char* format,...){
	va_list ptr;
	va_start(ptr,format);
	int r = vsprintf(buf,format,ptr);
	va_end(ptr);
	return r;
}
