#include <stdlib.h>
#include <sys/syscall.h>

extern cxa_atexit_handler cxa_atexit_handlers[MAX_CXA_HANDLERS];
extern uint16_t cxa_atexit_last_handler;

 void exit(int status){
	 while(cxa_atexit_last_handler){
		
		cxa_atexit_last_handler--; 
		cxa_atexit_handlers[cxa_atexit_last_handler].handl(cxa_atexit_handlers[cxa_atexit_last_handler].arg);
	 }
	 
	 sys_exit(0);
 }
