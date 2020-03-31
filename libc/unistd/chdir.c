#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <kernel/misc/pathutils.h>

int   chdir(char* path){
	if(path[0] == '/'){
		//Already absolute
		return sys_chdir(path);
	}
	char* curpath = malloc(256);
	curpath = getcwd(curpath,256);
	char* part = strtok(path,"/");
	if(!curpath){
		return -1;
	}
	while(part){
		//This wastes LOTS of memory
		if(!strcmp(part,"..") && strcmp(curpath,"/")){
			
			char* last = path_block(curpath,path_size(curpath)-1);
			uint32_t l = strlen(last);
			curpath = substr(curpath,0,strlen(curpath)-l-1);
			curpath = realloc(curpath,256); 
		}else if(strcmp(part,".")){
			curpath = strcat(curpath,part);
			curpath = strcat(curpath,"/");
		}
		part = strtok(NULL,"/");
	}
	return sys_chdir(curpath);
}
