#include <sys/syscall.h>

extern int main(int argc,char** argv, char** envp);
extern char** environ;
extern uint32_t env_size;

void libc_init(int argc, char** argv, char** envp){
	
	fix_user_heap();
	
	environ = malloc(sizeof(char*));
	environ[0] = 0;
	env_size = 0;
	if(envp && getpid() != 1){
		uint32_t i = 0;
		while(envp[i]){
			environ[i] = envp[i];
			environ = realloc(environ,(i+1)*sizeof(char*));
			i++;
		}
		environ[i] = 0;
		env_size = i;
	}
	
	char path[64];
	memset(path,0,64);
	sprintf(path,"/proc/%d/stdout",getpid());
	sys_open(path,F_WRITE);
	memset(path,0,64);
	sprintf(path,"/proc/%d/stderr",getpid());
	sys_open(path,F_WRITE);
	memset(path,0,64);
	sprintf(path,"/proc/%d/stdin",getpid());
	sys_open(path,F_READ);
	int resp = main(argc,argv,envp);
	sys_sleep(2);
	sys_exit(resp);
}
