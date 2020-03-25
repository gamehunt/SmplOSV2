/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdint.h>
#include <kernel/interrupts/syscalls.h>
#include <sys/syscall.h>
#include <kernel/fs/vfs.h>

int main(){
	//while(1);
	char* init = "Init start";
	char* path = "/root/usr/startup.d";
	sys_call(SYS_ECHO,init,0,0,0,0);
	uint32_t node = sys_call(SYS_OPEN,path,0,0,0,0);
	if(node){
		fs_dirent_t* dir = sys_call(SYS_READDIR,node,0,0,0,0);
	}
	char* exec = "/root/usr/bin/shell.smp";
	sys_call(SYS_EXEC,exec,0,0,0,0);
	while(1);
	return 0;
}
