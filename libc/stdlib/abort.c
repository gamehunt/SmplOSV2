/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdlib.h>
#include <sys/syscall.h>
#include <kernel/misc/panic.h>

__attribute__((__noreturn__)) void abort(void){
	#ifdef __smplos_libk
		crash_info_t crash;
		crash.description = "abort() called";
		kpanic(crash);
	#else
		sys_exit(1);
	#endif
}
