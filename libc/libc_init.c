#include <sys/syscall.h>

extern int main(int argc,char** argv, char** envp);

void libc_init(int argc, char** argv, char** envp){
	
	fix_user_heap();
	
	//default std streams
	if(!sys_open("/dev/tty",F_WRITE)){
		sys_open("/dev/null",F_WRITE);
	}
	if(!sys_open("/dev/tty",F_WRITE)){
		sys_open("/dev/null",F_WRITE);
	}
	sys_open("/dev/null",F_READ);
	
	sys_exit(main(argc,argv,envp));
}
