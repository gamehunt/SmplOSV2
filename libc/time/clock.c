/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <time.h>
#ifdef __smplos_libk
#include <kernel/dev/pit.h>
#endif
clock_t clock(){
	#ifdef __smplos_libk
		return pit_system_ticks();
	#endif
}
