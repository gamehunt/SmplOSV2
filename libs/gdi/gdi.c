#include <gdi.h>

#include <stdio.h>
#include <sys/syscall.h>

static FILE* framebuffer;

uint32_t gdi_rgb2linear(uint8_t r,uint8_t g,uint8_t b){
	return r | (g << 8) | (b << 16) | (0 << 24);
}

void    gdi_pixel(int x,int y,uint32_t color){
	sys_write(framebuffer->fd,0,(y*1024+x)*4,4,&color);
}

uint8_t gdi_init(char* fb){
	framebuffer = fopen(fb,"");
	uint16_t args[] = {1024,768,0x20};
	sys_ioctl(framebuffer->fd,0x10,args);
	gdi_pixel(0,0,gdi_rgb2linear(0,255,0));
	return 0;
}
