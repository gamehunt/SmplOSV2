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
	
	//TODO this should be readen from some file
	execv("/usr/bin/sterm.smp",0);
	while(1);
	return 0;
}
