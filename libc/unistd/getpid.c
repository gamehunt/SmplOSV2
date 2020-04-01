#include <unistd.h>
#include <sys/syscall.h>

pid_t getpid(){
	return sys_getpid(); 
}
pid_t getppid(void){
	return sys_getppid();
}
