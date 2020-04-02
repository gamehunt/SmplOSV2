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

#define XRES 1024
#define YRES 768

int main(){
	char* path = "/dev/tty";
	uint32_t args[] = {(uint32_t)path,1024};
	FILE* fb = fopen("/dev/fb0","w");
	sys_pipe("/dev/tty",512);
	FILE* tty = fopen("/dev/tty","r+");
	if(!tty || !fb){
		return 1;
	}
	gdi_init("/dev/fb0",XRES,YRES);
	int x = 0;
	int y = 12;
	int yoffs = 17;
	uint8_t* buffer = malloc(512);
	while(1){
		sys_fswait(&tty->fd,1);
		//gdi_char('A',0,12,gdi_rgb2linear(255,255,255),gdi_rgb2linear(0,0,0));
		memset(buffer,0,512);
		uint32_t size = fread(buffer,1,512,tty);
		//sys_echo("Readen: ",size);
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
				gdi_char(0,x,y,gdi_rgb2linear(255,255,255),gdi_rgb2linear(0,0,0));
				continue;
			}
			if(c == '\n'){
				x = 0;
				y+=17;
				if(y >= YRES){
					sys_ioctl(fb->fd,0x20,&yoffs);
					yoffs+=17;
				}
				continue;
			}
			if(c == '\t'){
				x += 30;
				if(x >= XRES){
						x = 0;
						y+=17;
						if(y >= YRES){ //TODO move this block to separate function
							sys_ioctl(fb->fd,0x20,&yoffs);
							yoffs+=17;
						}
				}
				continue;
			}
			if(c == '\0'){
				continue;
			}
			
			gdi_char(c,x,y,gdi_rgb2linear(255,255,255),gdi_rgb2linear(0,0,0));
			x+=9;
			if(x >= 1024){
				x = 0;
				y+=17;
				if(y >= YRES){
					sys_ioctl(fb->fd,0x20,&yoffs);
					yoffs+=17; //TODO handle overflow
				}
			}
		}
	}
	return 0;
}
