#include <unistd.h>
#include <sys/syscall.h>

uid_t getuid(){
	return sys_getuid();
}

int   setuid(uid_t uid){
	return sys_setuid(uid);
}
