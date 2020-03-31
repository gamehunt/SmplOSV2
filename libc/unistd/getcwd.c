#include <unistd.h>
#include <sys/syscall.h>


char* getcwd(char* buff,int size){
	return sys_getcwd(buff,size);
}
