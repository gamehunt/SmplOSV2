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
	execv("/root/usr/bin/tty.smp",0);
	char* args[] = {"arg",0};
	execv("/root/usr/bin/shell.smp",args,0);
	sys_call(SYS_SIG,2,0,0,0,0);
	while(1);
	return 0;
}
