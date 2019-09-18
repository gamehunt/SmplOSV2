#include <time.h>
#ifdef __smplos_libk
#include <kernel/dev/rtc.h>
#endif
time_t time(time_t *tp){
	#ifdef __smplos_libk
		return rtc_current_time();
	#endif
}
