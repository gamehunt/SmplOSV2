#include <kernel/dev/eld.h>

static int dx,dy;

void eld_init(){
	memset(VGA_MEMORY,0,VGA_WIDTH*VGA_HEIGHT*2);
}

static void eld_scroll(){
	memmove(VGA_MEMORY, ((uint8_t*)VGA_MEMORY)+160, VGA_WIDTH*VGA_HEIGHT*2);
}

void eld_putchar(char c){
	serial_write(SERIAL_COM1,c);
	//return;
	if(c == '\n'){
		dx = 0;
		dy++;
		if(dy >= VGA_HEIGHT){
			dy--;
			eld_scroll();
		}
		return;
	}
	if(c == '\0'){
		return;
	}
	if(c=='\t'){
		dx += 3;
		if(dx >= VGA_WIDTH){
			dx = 0;
			dy++;
			if(dy >= VGA_HEIGHT){
				dy--;
				eld_scroll();
			}
		}
		return;
	}
	VGA_MEMORY[dy*VGA_WIDTH + dx] = vga_entry(c,vga_entry_color(VGA_COLOR_LIGHT_GREY,VGA_COLOR_BLACK));
	dx++;
	if(dx >= VGA_WIDTH){
		dx = 0;
		dy++;
		if(dy >= VGA_HEIGHT){
			dy--;
			eld_scroll();
		}
	}
}
void eld_puts(const char* s,int len){
	for(int i=0;i<len;i++){
		eld_putchar(s[i]);
	}
}
