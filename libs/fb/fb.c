#include <fb.h>
#include <fb_font.h>

#include <stdio.h>
#include <sys/syscall.h>

static FILE* framebuffer;
static uint16_t x_res;
static uint16_t y_res;
static uint32_t* back_buffer = 0;

static unsigned char lookup[16] = {
0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf, };

uint8_t reverse(uint8_t n) {
   // Reverse the top and bottom nibble then swap them.
   return (lookup[n&0b1111] << 4) | lookup[n>>4];
}

uint32_t fb_rgb2linear(uint8_t r,uint8_t g,uint8_t b){
	return b | (g << 8) | (r << 16) | (0 << 24);
}

void    fb_pixel(int x,int y,uint32_t color){
	if( x < 0 || x > x_res || y < 0 || y > y_res){
		return;
	}
	if(framebuffer){
		if(!back_buffer){
			sys_write(framebuffer->fd,(y*x_res+x)*4,4,&color);
		}else{
			back_buffer[y*x_res+x] = color;
		}
	}
}

void fb_swapbuffers(){
	if(back_buffer){
		sys_write(framebuffer->fd,0,2*1024*768*sizeof(uint32_t),back_buffer);
	}
}

void    fb_char(unsigned char c,int x,int y,uint32_t fc,uint32_t bc){
	uint8_t mask[8];
	for(int i=0;i<8;i++){
		mask[i] = (i==0?1:mask[i-1]*2);
	}
	for(int i=0;i<16;i++){
		for(int j=0;j<8;j++){
			fb_pixel(x+j,y+i,reverse(ibmvga_8x16_font[c*16+i])&mask[j]?fc:bc);
		}
	}
}

uint8_t fb_init(char* fb,uint16_t xres,uint16_t yres,uint8_t double_buffer){
	x_res = xres;
	y_res = yres;
	framebuffer = fopen(fb,"w");
	if(!framebuffer){
		printf("Failed to open framebuffer!\n");
		return 1;
	}
	if(double_buffer){
		back_buffer = malloc(2*1024*768*sizeof(uint32_t));
		
	}
	return 0;
}

void fb_fill(int x,int y,int sx,int sy,uint32_t color){
	for(int i=0;i<sx;i++){
		for(int j=0;j<sy;j++){
			fb_pixel(x+i,y+j,color);
		}
	}
}
