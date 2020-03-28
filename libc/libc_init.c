#include <sys/syscall.h>

extern int main(int argc,char** argv);

void libc_init(int argc, char** argv){
	//default std streams
	sys_open("/dev/tty");
	sys_open("/dev/tty");
	sys_open("/dev/null");
	sys_exit(main(argc,argv));
}
