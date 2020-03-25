/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdint.h>
#include <sys/syscall.h>
#include <kernel/fs/vfs.h>

int main(){
	//while(1);
	char* path = "/root/usr/startup.d";
	char* fork = "FORK";
	char* par  = "PAR";
	uint32_t node = sys_call(3,path,0,0,0,0);
	if(node){
		fs_dirent_t* dir = sys_call(4,node,0,0,0,0);
	}
	uint32_t parent = sys_call(10,0,0,0,0,0);
	if(!parent){
		//sys_call(0,fork,0,0,0,0);
		char* exec = "/root/usr/bin/shell.smp";
		sys_call(5,exec,0,0,0,0);
	}
	while(1);
	return 0;
}
