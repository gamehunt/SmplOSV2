/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdint.h>
#include <sys/syscall.h>
#include <kernel/fs/vfs.h>

void _start(){
	char* path = "/root/usr/startup.d";
	fs_dirent_t* node = sys_call(4,path,0,0,0,0);
	char* exec = "/root/usr/bin/shell.smp";
	sys_call(5,exec,0,0,0,0);
	while(1);
}
