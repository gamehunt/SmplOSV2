/*	

    Copyright (C) 2020
     
    Author: gamehunt 


	Terminal emulator. Acts as a mini-server for console applications and shell
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

#define CELL_SZ_X 9
#define CELL_SZ_Y 20

uint16_t tx,ty;
uint32_t term_col_bg;
uint32_t term_col_fg;

FILE* second_out;
FILE* second_in;

char input_buffer[256];
uint32_t input_size = 0;

FILE* fb;

int sig_child(){
	exit(0);
}

void term_scroll(){
	uint32_t offs = CELL_SZ_Y;
	sys_ioctl(fb->fd,0x20,&offs); //This should be in compositor
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
	}else{
		gdi_char(c,tx*CELL_SZ_X,ty*CELL_SZ_Y,term_col_fg,term_col_bg);
		tx++;
	}
	
	if(tx >= XRES/CELL_SZ_X){
		tx = 0;
		ty++;
	}
	if(ty >= YRES/CELL_SZ_Y){
		ty--;
		term_scroll();
	}
}

int term_init(int argc,char** argv){
	tx = 0;
	ty = 1;
	fb = fopen("/dev/fb0","w");
	if(!fb){
		return 1;
	}
	gdi_init("/dev/fb0",XRES,YRES);
	term_col_bg = gdi_rgb2linear(0,0,0);
	term_col_fg = gdi_rgb2linear(255,255,255);
	sys_signal(SIG_CHILD,sig_child);
	uint32_t shell_exec = execv(argc?argv[0]:"/usr/bin/shell.smp",0);
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
	return 0;
	
}

void term_handle_input(char c){
	if(c==0x08){
		input_size--;
		input_buffer[input_size] = 0;
		if(!tx){
			tx = XRES;
			ty--;
		}else{
			tx--;
		}
		gdi_char(0,tx*CELL_SZ_X,ty*CELL_SZ_Y,term_col_fg,term_col_bg);
		return;
	}
	term_putchar(c);
	input_buffer[input_size] = c;
	input_size++;
	if(c =='\n' && second_out){
		fwrite(input_buffer,1,input_size,second_out);
		memset(input_buffer,0,input_size);
		input_size = 0;
	}
}

int main(int argc, char** argv){
	if(term_init(argc,argv)){
		return 1;
	}
	char* buffer = malloc(256);
	while(1){
		uint32_t readen = fread(buffer,1,256,stdin);
		for(uint32_t i=0;i<readen;i++){
				term_handle_input(buffer[i]);
		}
		readen = fread(buffer,1,256,second_in);
		for(uint32_t i=0;i<readen;i++){
			term_putchar(buffer[i]);
		}
	}
	return 0;
}
