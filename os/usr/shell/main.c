/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdio.h>
#include <kernel/interrupts/syscalls.h>
#include <sys/syscall.h>
#include <kernel/fs/vfs.h>

int main(){
	//while(1);
	printf("HUY\n");
	sys_echo("Launched shell",0);
	FILE* kbd = fopen("/dev/kbd","");
	while(1){
		sys_fswait(&kbd->fd,1);
		uint8_t kc;
		sys_read(kbd->fd,0,1,&kc);
		sys_echo("Keycode received",kc);
	}
	//while(1);
}
