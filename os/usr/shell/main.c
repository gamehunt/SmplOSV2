/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdint.h>
#include <sys/syscall.h>
#include <kernel/fs/vfs.h>

void _start(){
	char* out = "Launched shell";
	sys_call(0,out,0,0,0,0);
	sys_call(7,0,0,0,0,0);
	while(1);
}
