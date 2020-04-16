#include <unistd.h>
#include <sys/syscall.h>

int getpriority(int which, int who){
	return sys_getprior(which,who);
}
int setpriority(int which, int who, int prio){
	return sys_setprior(which,who,prio);
}
