/*	

    Copyright (C) 2020
     
    Author: gamehunt 


	Terminal emulator. Acts as a mini-server for console applications and shell
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <unistd.h>
#include <kernel/interrupts/syscalls.h>
#include <sys/syscall.h>
#include <kernel/fs/vfs.h>
#include <cserv/cserv.h>
#include <fb.h>

#define XRES 1024
#define YRES 768

#define CELL_SZ_X 9
#define CELL_SZ_Y 20

#define X_CELL_RES (XRES/CELL_SZ_X)
#define Y_CELL_RES (YRES/CELL_SZ_Y)

uint16_t tx;
uint16_t ty;

uint32_t term_col_bg;
uint32_t term_col_fg;

FILE* second_out;
FILE* second_in;

char input_buffer[256];
uint32_t input_size = 0;

void term_scroll(){
	uint32_t* plain = CServer::C_GetContext()->GetCanvas();
	memmove(plain, plain+XRES*CELL_SZ_Y, XRES*(YRES)*4);
	fb_rect(0,CELL_SZ_Y*(Y_CELL_RES-1),XRES,CELL_SZ_Y,term_col_bg,true,plain);
}

void term_putchar(char c){
		if(c == '\n'){
			tx = 0;
			ty++;
			if(ty >= Y_CELL_RES){
				ty--;
				term_scroll();
			}
		}else{
			fb_optbuff_size(XRES,YRES);
			uint32_t* plain = CServer::C_GetContext()->GetCanvas();
			fb_char(c,tx*CELL_SZ_X,ty*CELL_SZ_Y,term_col_fg,term_col_bg,plain);
			tx++;
			if(tx >= X_CELL_RES){
				tx = 0;
				ty++;
			}
			if(ty >= Y_CELL_RES){
				ty--;
				term_scroll();
			}
	}
}

int sig_child(){
	fclose(second_out);
	if(fread(input_buffer,1,256,second_in)){
		for(uint32_t i=0;i<256;i++){
			if(!input_buffer[i]){
				break;
			}
			term_putchar(input_buffer[i]);
		}
	}
	
	exit(0);
}

int term_init(int argc,char** argv){
	CServer::C_InitClient(XRES,YRES);
	fb_optbuff_size(XRES,YRES);
	uint32_t* ctx = CServer::C_GetContext()->GetCanvas();
	
	term_col_bg = fb_rgb2linear(0,0,0x8B);
	term_col_fg = fb_rgb2linear(255,255,255);
	
	fb_rect(0,0,XRES,YRES,term_col_bg,true,ctx);

	tx = 0;
	ty = 0;
	

	
	sys_signal(SIG_CHILD,sig_child);
	uint32_t shell_exec = execv(argc?argv[0]:"/usr/bin/shell.smp",0);
	sys_echo("shell_exec: %d\n",shell_exec);
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
	return shell_exec?0:1;
	
}

void term_handle_input(char c){
	if(c==0x08 && input_size){
		input_size--;
		input_buffer[input_size] = 0;
		if(!tx && ty){
			tx = X_CELL_RES-1;
			ty--;
		}else{
			tx--;
		}
		fb_optbuff_size(XRES,YRES);
		uint32_t* plain = CServer::C_GetContext()->GetCanvas();
		fb_char(0,tx*CELL_SZ_X,ty*CELL_SZ_Y,term_col_fg,term_col_bg,plain);
		return;
	}else{
		term_putchar(c);
		input_buffer[input_size] = c;
		input_size++;
	}
	if(c =='\n' && second_out){
		fwrite(input_buffer,1,input_size,second_out);
		memset(input_buffer,0,input_size);
		input_size = 0;
	}
}

int main(int argc, char** argv){
	if(term_init(argc,argv)){
		sys_echo("Failed to initialize terminal!",0);
		return 1;
	}
	sys_echo("Terminal initialized!\n");
	char* buffer = (char*)malloc(256);
	
	uint32_t* fds = new uint32_t[2]{stdin->fd,second_in->fd};
	
	while(1){
		memset(buffer,0,256);
		uint32_t readen = fread(buffer,1,256,stdin);
		for(uint32_t i=0;i<readen;i++){	
				term_handle_input(buffer[i]);
		}
		memset(buffer,0,256);
		readen = fread(buffer,1,256,second_in);
		for(uint32_t i=0;i<readen;i++){
			term_putchar(buffer[i]);
		}
		
	}
	return 0;
}
