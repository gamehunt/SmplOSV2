#include <unistd.h>
#include <sys/syscall.h>

extern char **environ;

pid_t fork(void){
	printf("fork() called, aborting\n");
	abort(); //We haven't fork
	return 0;
}


int execve(const char *filename, char *const argv [], char *const envp[]){
	
	return sys_exec(filename,argv,envp);; //TODO err code? success?
}

int execvp(const char* f, char* const argv[]){
	//TODO getenv and search
	return 0; //TODO err code? success?
}
int execv(const char* f, char* const argv[]){
	return (int)sys_exec(f,argv,environ);;
}
