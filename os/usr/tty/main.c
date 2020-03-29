/*	

    Copyright (C) 2020
     
    Author: gamehunt 
     
    Simple terminal emulator for graphic modes, just draws keys from tty pipe

*/

#include <stdio.h>
#include <kernel/interrupts/syscalls.h>
#include <sys/syscall.h>
#include <kernel/fs/vfs.h>

#include <gdi.h>

int main(){
	char* path = "/dev/tty";
	uint32_t args[] = {(uint32_t)path,1024};
	FILE* master_pipe = fopen("/dev/pipe","");
	sys_ioctl(master_pipe->fd,0xC0,args);
	sys_close(master_pipe->fd);
	FILE* tty = fopen("/dev/tty","");
	//gdi_init("/dev/fb0",1024,768);
	int x = 0;
	int y = 12;
	uint8_t* buffer = malloc(1024);
	//TODO scrolling
	while(1){
		sys_fswait(&tty->fd,1);
		memset(buffer,0,1024);
		uint32_t size = sys_read(tty->fd,0,1024,buffer);
		for(uint32_t i=0;i<size;i++){
			uint8_t c = buffer[i];
			if(c == 0x8){ //backspace
				x-=9;
				if(x < 0){
					x = 0;
					y-=17;
					if(y<0){
						y=0;
					}
				}
				//gdi_char(0,x,y,gdi_rgb2linear(255,255,255),gdi_rgb2linear(0,0,0));
				continue;
			}
			if(c == '\n'){
				x = 0;
				y+=17;
				continue;
			}
			if(c == '\t'){
				x += 30;
				if(x >= 1024){
						x = 0;
						y+=17;
					}
					continue;
			}
			if(c == '\0'){
				continue;
			}
			
			//gdi_char(c,x,y,gdi_rgb2linear(255,255,255),gdi_rgb2linear(0,0,0));
			x+=9;
			if(x >= 1024){
				x = 0;
				y+=17;
			}
		}
	}
	return 0;
}
