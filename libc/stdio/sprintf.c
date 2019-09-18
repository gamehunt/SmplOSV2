#include <stdio.h>

void sprintf(char* buf,const char* format,...){
	va_list ptr;
	va_start(ptr,format);
	vsprintf(buf,format,ptr);
	va_end(ptr);
}
