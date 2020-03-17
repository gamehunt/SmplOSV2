/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdio.h>
#ifdef __smplos_libk
#include <kernel/dev/eld.h>
#endif

void puts(const char* str){
	#ifdef __smplos_libk
	eld_puts(str,strlen(str));
	#endif
}
