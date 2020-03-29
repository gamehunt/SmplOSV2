/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <time.h>

#ifdef __smplos_libk
#include <kernel/dev/rtc.h>
#else
#include <sys/syscall.h>
#endif
time_t time(time_t *tp){
	#ifdef __smplos_libk
		return rtc_current_time();
	#else
		uint32_t res = sys_time();
		if(tp){
			*tp = res;
		}
		return res;
	#endif
		
}
