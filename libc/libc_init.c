#include <sys/syscall.h>

extern int main(int argc,char** argv, char** envp);
extern char** environ;
extern uint32_t env_size;

void libc_init(int argc, char** argv, char** envp){
	
	fix_user_heap();
	
	if(!environ){
		environ = malloc(sizeof(char*));
		environ[0] = 0;
	}
	uint32_t i = 0;
	while(envp[i]){
		environ[i] = envp[i];
		environ = realloc(environ,(i+1)*sizeof(char*));
		i++;
	}
	environ[i] = 0;
	env_size = i;
	
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
