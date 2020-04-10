#include <stdlib.h>
#include <sys/syscall.h>




cxa_atexit_handler cxa_atexit_handlers[MAX_CXA_HANDLERS];
uint16_t cxa_atexit_last_handler = 0;

int __cxa_atexit(atexit_handl_t handl, void * arg, void * dso_handle){
	if(MAX_CXA_HANDLERS <= cxa_atexit_last_handler){
		return 1;
	}
	cxa_atexit_handlers[cxa_atexit_last_handler].handl = handl;
	cxa_atexit_handlers[cxa_atexit_last_handler].arg = arg;
	cxa_atexit_last_handler++;
	return 0;
}
