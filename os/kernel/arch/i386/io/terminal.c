#include <kernel/io/terminal.h>
//#include <kernel/debug/debug.h>
#include <kernel/video/vga.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;
static size_t ty;
static size_t tx;
static uint8_t terminal_col;
static uint8_t terminal_b;
static uint16_t* vmem = VGA_MEMORY;

void terminal_init(){

	terminal_entry(0,0);
	terminal_color(VGA_COLOR_WHITE);
	terminal_back(VGA_COLOR_BLACK);
	for(int i = 0; i < VGA_HEIGHT; i++){
        	for (int m = 0; m < VGA_WIDTH; m++){
           	 	vmem[i * VGA_WIDTH + m] = 0;
       		}
    	}
	
}

void terminal_entry(int x,int y){
	tx = x;
	ty = y;
}
void terminal_scroll(){
    for(int i = 0; i < VGA_HEIGHT; i++){
        for (int m = 0; m < VGA_WIDTH; m++){
            vmem[i * VGA_WIDTH + m] = vmem[(i + 1) * VGA_WIDTH + m];
        }
    }
}
void terminal_putchar(char c){
	const size_t index = ty * VGA_WIDTH + tx;
	if(c == '\n'){
		ty++;
		tx=0;
	}else if(c == '\t'){
		tx+=3;
	}else{
		vmem[index] = vga_entry(c,vga_entry_color(terminal_col,terminal_b));
		tx++;
	}
	if(tx >= VGA_WIDTH){
		tx = 0;
		ty++;
	}
	while(ty >= VGA_HEIGHT){
		terminal_scroll();
		ty--;
	}
	__update_cursor();
}

void terminal_color(enum vga_color c){
	terminal_col = c;
}

void terminal_back(enum vga_color c){
	terminal_b = c;
}

void terminal_clear(){
    for(int i = 0; i < VGA_HEIGHT; i++){
        for (int m = 0; m < VGA_WIDTH; m++){
            vmem[i * VGA_WIDTH + m] = vga_entry(' ',vga_entry_color(terminal_col,terminal_b));
        }
    }
   	terminal_entry(0,0);
}

void __update_cursor(){
	uint16_t pos = ty * VGA_WIDTH + tx;
 
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

