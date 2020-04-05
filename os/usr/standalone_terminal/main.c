/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <kernel/interrupts/syscalls.h>
#include <sys/syscall.h>
#include <kernel/fs/vfs.h>
#include <gdi.h>
#include <kbd.h>

#define XRES 1024
#define YRES 768

#define CELL_SZ_X 10
#define CELL_SZ_Y 20

uint16_t tx,ty;
uint32_t term_col_bg;
uint32_t term_col_fg;

FILE* second_out;
FILE* second_in;

void term_scroll(){
	
}

void term_putchar(char c){
	if(c=='\n'){
		ty++;
		tx = 0;
	}
	else if(c=='\t'){
		tx+=3;
	}
	else if(c=='\0'){
		return;
	}
	else if(c==0x08){
		if(!tx){
			tx = XRES;
			ty--;
		}else{
			tx--;
		}
		term_putchar(0);
	}else{
		gdi_char(c,tx*CELL_SZ_X,ty*CELL_SZ_Y,term_col_fg,term_col_bg);
		tx++;
	}
	
	if(tx >= XRES){
		tx = 0;
		ty++;
	}
	if(ty >= YRES){
		ty--;
		term_scroll();
	}
}

void term_init(){
	tx = 0;
	ty = 1;
	gdi_init("/dev/fb0",XRES,YRES);
	term_col_bg = gdi_rgb2linear(0,0,0);
	term_col_fg = gdi_rgb2linear(255,255,255);
	sys_close(0);
	sys_close(2);
	uint32_t shell_exec = execv("/usr/bin/shell.smp",0);
	if(shell_exec){
		char buffer[64];
		char buffer1[64];
		memset(buffer,0,64);
		memset(buffer1,0,64);
		sprintf(buffer,"/proc/%d/stdin",shell_exec);
		sprintf(buffer1,"/proc/%d/stdout",shell_exec);
		while(!(second_out = fopen(buffer,"w")));
		while(!(second_in  = fopen(buffer1,"r")));
	}
	
}

void term_handle_input(char c){
	term_putchar(c);
	if(c != 0x08 && second_out){
		fwrite(&c,1,1,second_out);
	}
	if(second_out){
		putchar(c);
	}
}

int main(){
	term_init();
	FILE* kbd_in = fopen("/dev/kbd","r");
	key_t* key = malloc(sizeof(key_t));
	char* buffer = malloc(256);
	while(1){
		uint32_t readen = fread(buffer,1,256,kbd_in);
		for(uint32_t i=0;i<readen;i++){
			kbd_key_event(key,buffer[i]);
			if(key->state != KEY_ACTION_UP){
				term_handle_input(key->key);
			}
		}
		readen = fread(buffer,1,256,stdin);
		for(uint32_t i=0;i<readen;i++){
			term_putchar(buffer[i]);
		}
	}
	return 0;
}
