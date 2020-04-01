#include <stdlib.h>
#include <sys/syscall.h>


 void exit(int status){
	 sys_exit(0);
 }
