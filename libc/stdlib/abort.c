#include <stdlib.h>
#include <kernel/misc/panic.h>

__attribute__((__noreturn__)) void abort(void){
	#ifdef __smplos_libk
		crash_info_t crash;
		crash.description = "abort() called";
		kpanic(crash);
	#endif

	while(1);
}
