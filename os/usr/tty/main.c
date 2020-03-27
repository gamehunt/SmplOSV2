/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdio.h>
#include <kernel/interrupts/syscalls.h>
#include <sys/syscall.h>
#include <kernel/fs/vfs.h>

#include <gdi.h>

int main(){
	char* path = "/dev/tty";
	uint32_t args[] = {(uint32_t)path,512};
	sys_ioctl("/dev/pipe",0xC0,args);
	FILE* tty = sys_open("/dev/tty");
	gdi_init("/dev/fb0");
	while(1);
	return 0;
}
