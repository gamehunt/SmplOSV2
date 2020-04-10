#include <stdlib.h>

extern int __cxa_atexit(void (*func) (void *), void * arg, void * dso_handle);

int atexit(void (*func)(void)){
	return __cxa_atexit(func,NULL,NULL);
}
