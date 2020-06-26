#include <fb.h>
#include <fb_font.h>

#include <stdio.h>
#include <sys/syscall.h>

static FILE* framebuffer;
static uint16_t x_res;
static uint16_t y_res;
static uint16_t x_opt_res;
static uint16_t y_opt_res;
static uint32_t* back_buffer = 0;
static uint8_t back_buffer_enabled = 0;

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

void    fb_pixel(int x,int y,uint32_t color,uint32_t* opt_buffer){
	if((x < 0 || x > x_res || y < 0 || y > y_res) && !opt_buffer){
		return;
	}
	if(framebuffer || opt_buffer){
		if(!back_buffer_enabled && !opt_buffer){
			__memcpy_opt(0xFD000000+(y*x_res+x)*4,&color,1);
		}else{
			uint32_t* bfr = opt_buffer?opt_buffer:back_buffer;
			uint16_t res = opt_buffer?x_opt_res:x_res;
			bfr[y*res+x] = color;
		}
	}
}

void __memcpy_opt(uint32_t* dest,uint32_t* src,uint32_t size){
	for(uint32_t i=0;i<size;i++){
		dest[i] = src[i];
	}
}

void __memset_opt(uint32_t* dest,uint32_t value,uint32_t size){
	for(uint32_t i=0;i<size;i++){
		dest[i] = value;
	}
}

void fb_swapbuffers(){
	if(back_buffer_enabled){
		if(back_buffer != 0xFD000000){
			back_buffer = 0xFD000000;
			uint32_t* arg = malloc(sizeof(uint32_t));
			*arg = y_res;
			sys_ioctl(framebuffer,0x20,arg);
			*arg = 0;
			sys_ioctl(framebuffer,0x30,arg);
			free(arg);
		}else{
			//sys_echo("doing nothing...\n");
			back_buffer = 0xFD000000 + x_res*y_res*4;
			uint32_t* arg = malloc(sizeof(uint32_t));
			*arg = 0;
			sys_ioctl(framebuffer,0x20,arg);
			*arg = 0;
			sys_ioctl(framebuffer,0x30,arg);
			free(arg);
		}
	}
}

void    fb_char(unsigned char c,int x,int y,uint32_t fc,uint32_t bc,uint32_t* opt_buffer){
	uint8_t mask[8];
	for(int i=0;i<8;i++){
		mask[i] = (i==0?1:mask[i-1]*2);
	}
	for(int i=0;i<16;i++){
		for(int j=0;j<8;j++){
			fb_pixel(x+j,y+i,reverse(ibmvga_8x16_font[c*16+i])&mask[j]?fc:bc,opt_buffer);
		}
	}
}

uint8_t fb_init(char* fb,uint16_t xres,uint16_t yres,uint8_t double_buffer){
	x_res = xres;
	y_res = yres;
	x_opt_res = 0;
	y_opt_res = 0;
	framebuffer = fopen(fb,"w");
	if(!framebuffer){
		printf("Failed to open framebuffer!\n");
		return 1;
	}
	back_buffer_enabled = double_buffer;
	back_buffer = 0xFD000000 + x_res*y_res*4;
	return 0;
}

//Bresenham algo
void    fb_line(int x0,int y0, int x1,int y1,uint32_t color,uint32_t* opt_buffer){
	if(x0 == x1){
		
		for(int i=y0;i<=y1;i++){
			fb_pixel(x0,i,color,opt_buffer);
		}
		
		return;
	}
	 int deltax = abs(x1 - x0);
     int deltay = abs(y1 - y0);
     int error = 0;
     int deltaerr = (deltay + 1);
     int y = y0;
     int diry = y1 - y0;
     if (diry > 0) {
         diry = 1;
     }
     if (diry < 0) {
         diry = -1;
     }
     for(int x = x0; x<=x1;x++){
         fb_pixel(x,y,color,opt_buffer);
         error += deltaerr;
         if(error >= (deltax + 1)){
             y += diry;
             error -= (deltax + 1);
         }
     }
}

void    fb_circle(int X1,int Y1,int R,uint32_t color,uint8_t fill,uint32_t* opt_buffer){
   int x = 0;
   int y = R;
   int delta = 1 - 2 * R;
   int error = 0;
   while (y >= 0){
	   if(!fill){
			fb_pixel(X1 + x, Y1 + y, color,opt_buffer);
			fb_pixel(X1 + x, Y1 - y, color,opt_buffer);
			fb_pixel(X1 - x, Y1 + y, color,opt_buffer);
			fb_pixel(X1 - x, Y1 - y, color,opt_buffer);
	   }else{
		    fb_line(X1 - x, Y1 + y,X1 + x,Y1 + y,color,opt_buffer);
		    fb_line(X1 - x, Y1 - y,X1 + x,Y1 - y,color,opt_buffer);
	   }
       error = 2 * (delta + y) - 1;
       if ((delta < 0) && (error <= 0)){
           delta += 2 * (++x) + 1;
           continue;
       }
       if ((delta > 0) && (error > 0)){
           delta -= 2 * (--y) + 1;
           continue;
       }
       delta += 2 * (++x - y--);
	}
}

void fb_rect(int x,int y,int sx,int sy,uint32_t color,uint8_t filled,uint32_t* opt_buffer){
	if(filled){
		
		for(int i=0;i<sy;i++){
			
			if(!back_buffer_enabled && !opt_buffer){
				//TODO
			}else{
				int offset = (opt_buffer?x_opt_res:x_res) * (y + i) + x;
				uint32_t* bfr = (opt_buffer?opt_buffer:back_buffer);
				__memset_opt(&bfr[offset],color,sx);
			}
		
		}
	}else{
		fb_line(x,y,x+sx,y,color,opt_buffer);
		fb_line(x+sx,y,x+sx,y+sy,color,opt_buffer);
		fb_line(x,y,x,y+sy,color,opt_buffer);
		fb_line(x,y+sx,x+sx,y+sx,color,opt_buffer);
	}
}

void    fb_triangle(int x,int y,int x1,int y1,int x2,int y2,uint32_t color,uint8_t fill,uint32_t* opt_buffer){
	if(!fill){
		fb_line(x,y,x1,y1,color,opt_buffer);
		fb_line(x1,y1,x2,y2,color,opt_buffer);
		fb_line(x,y,x2,y2,color,opt_buffer);
	}else{
		//TODO
	}
}

void    fb_inject_buffer(int x,int y,int bsx,int bsy,uint32_t* buffer,uint32_t* opt_buffer){
	for(int i=0;i<bsy;i++){
		uint16_t res = opt_buffer?x_opt_res:x_res;
		int offset = res * (y + i) + x;
		if(!back_buffer_enabled && !opt_buffer){
			//TODO
		}else{
			uint32_t* buf = opt_buffer?opt_buffer:back_buffer;
			__memcpy_opt(&buf[offset],&buffer[i*bsx],bsx);
			//if(opt_buffer){
			//	sys_echo("Copied for opt_buffer\n");
			//}
		}
		
	}
}

void    fb_optbuff_size(int x,int y){
	x_opt_res = x;
	y_opt_res = y;
}
