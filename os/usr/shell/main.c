/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdint.h>
#include <sys/syscall.h>
#include <kernel/fs/vfs.h>

void _start(){
	char* out = "Launched shell";
	char* path = "/dev/kbd";
	sys_call(0,out,0,0,0,0);
	fs_node_t* kbd = sys_call(3,path,0,0,0,0);
	while(1){
		char keycode = 0;
		while(!sys_call(1,(uint32_t)kbd,0,0,1,(uint32_t)&keycode));
		char str[12] = "Keycode: ";
		sys_call(0,str,keycode,0,0,0);
	}
	sys_call(7,0,0,0,0,0);
	while(1);
}
