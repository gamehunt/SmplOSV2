/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdint.h>
#include <kernel/interrupts/syscalls.h>
#include <sys/syscall.h>
#include <kernel/fs/vfs.h>

int main(){
	//*((uint8_t*)0xFFFFFFFF) = 0;
	sys_echo("Init start",0);
	uint32_t node = sys_open("/root/usr/startup.d");
	if(node){
		fs_dirent_t* dir = sys_readdir(node);
	}
	sys_exec("/root/usr/bin/shell.smp",0,0,0);
	while(1);
	return 0;
}
