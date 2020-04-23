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
			__memcpy_opt(0xFD000000+(y*x_res+x)*4,&color,1);
		}else{
			back_buffer[y*x_res+x] = color;
		}
	}
}

void __memcpy_opt(uint32_t* dest,uint32_t* src,uint32_t size){
	for(uint32_t i=0;i<size;i++){
		dest[i] = src[i];
	}
}

void fb_swapbuffers(){
	if(back_buffer){
		__memcpy_opt(0xFD000000,back_buffer,1024*768);
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

//Bresenham algo
void    fb_line(int x0,int y0, int x1,int y1,uint32_t color){
	if(x0 == x1){
		
		for(int i=y0;i<=y1;i++){
			fb_pixel(x0,i,color);
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
         fb_pixel(x,y,color);
         error += deltaerr;
         if(error >= (deltax + 1)){
             y += diry;
             error -= (deltax + 1);
         }
     }
}

void    fb_circle(int X1,int Y1,int R,uint32_t color,uint8_t fill){
   int x = 0;
   int y = R;
   int delta = 1 - 2 * R;
   int error = 0;
   while (y >= 0){
	   if(!fill){
			fb_pixel(X1 + x, Y1 + y, color);
			fb_pixel(X1 + x, Y1 - y, color);
			fb_pixel(X1 - x, Y1 + y, color);
			fb_pixel(X1 - x, Y1 - y, color);
	   }else{
		    fb_line(X1 - x, Y1 + y,X1 + x,Y1 + y,color);
		    fb_line(X1 - x, Y1 - y,X1 + x,Y1 - y,color);
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

void fb_rect(int x,int y,int sx,int sy,uint32_t color,uint8_t filled){
	if(filled){
		for(int j=0;j<sy;j++){
			fb_line(x,y+j,x+sx,y+j,color);
		}
	}else{
		fb_line(x,y,x+sx,y,color);
		fb_line(x+sx,y,x+sx,y+sy,color);
		fb_line(x,y,x,y+sy,color);
		fb_line(x,y+sx,x+sx,y+sx,color);
	}
}

void    fb_triangle(int x,int y,int x1,int y1,int x2,int y2,uint32_t color,uint8_t fill){
	if(!fill){
		fb_line(x,y,x1,y1,color);
		fb_line(x1,y1,x2,y2,color);
		fb_line(x,y,x2,y2,color);
	}else{
		//TODO
	}
}
