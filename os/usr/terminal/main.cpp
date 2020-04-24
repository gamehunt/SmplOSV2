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

char screen_buffer[1024*768/(CELL_SZ_X*CELL_SZ_Y)];

void term_scroll(){
	uint32_t offs = CELL_SZ_Y;
	//Call some function from compositor to scroll down our context
}

void term_putchar(char c){
	screen_buffer[tx*X_CELL_RES + ty] = c;
	tx++;
	if(tx >= X_CELL_RES){
		tx = 0;
		ty++;
	}
	if(ty >= Y_CELL_RES){
		//TODO
	}
	CSPacket* p = new CSPacket(CS_TYPE_WIDGET);
	((uint32_t*)p->GetBuffer())[0] = getpid();
	((uint32_t*)p->GetBuffer())[1] = WIDGET_PACK_UPD;
	((uint32_t*)p->GetBuffer())[2] = 0;
	std::memcpy((char*)(&(((uint32_t*)p->GetBuffer())[3])),screen_buffer,128);
	CServer::C_SendPacket(p);
	delete p;
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
	CServer::C_InitClient();
	CSPacket* p = new CSPacket(CS_TYPE_WIDGET);
	((uint32_t*)p->GetBuffer())[0] = getpid();
	((uint32_t*)p->GetBuffer())[1] = WIDGET_PACK_ADD;
	((uint32_t*)p->GetBuffer())[2] = 1;
	CServer::C_SendPacket(p);
	((uint32_t*)p->GetBuffer())[1] = WIDGET_PACK_RES;
	((uint32_t*)p->GetBuffer())[2] = 0;
	((uint32_t*)p->GetBuffer())[3] = 1024;
	((uint32_t*)p->GetBuffer())[4] = 768;
	CServer::C_SendPacket(p);
	tx = 0;
	ty = 1;
	term_col_bg = fb_rgb2linear(0,0,0);
	term_col_fg = fb_rgb2linear(255,255,255);

	term_putchar('A');
	term_putchar('B');
	term_putchar('C');

	sys_signal(SIG_CHILD,sig_child);
	uint32_t shell_exec = execv(argc?argv[0]:"/usr/bin/shell.smp",0);
	//sys_echo("shell_exec: ",shell_exec);
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
	if(c==0x08){
		input_size--;
		input_buffer[input_size] = 0;
		if(!tx){
			tx = XRES;
			ty--;
		}else{
			tx--;
		}
		screen_buffer[tx*X_CELL_RES + ty] = 0;
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
		sys_echo("Failed to initialize terminal!",0);
		return 1;
	}
	char* buffer = (char*)malloc(256);
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
