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
	sys_echo("Launched shell",0);
	uint32_t kbd = sys_open("/dev/kbd");
	while(1){
		sys_fswait(&kbd,1);
		uint8_t kc;
		sys_read(kbd,0,1,&kc);
		sys_echo("Keycode received",kc);
	}
	sys_exit(0);
	while(1);
}
