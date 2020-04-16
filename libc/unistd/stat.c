#include <unistd.h>
#include <sys/syscall.h>

int stat(int fd,stat_t* stat){
	return sys_stat(fd,stat);
}
