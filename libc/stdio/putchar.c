/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdio.h>
#ifdef __smplos_libk
	#include<kernel/dev/eld.h>
#endif
void putchar(char c){
	#ifdef __smplos_libk
	eld_putchar(c);
	#endif
}
