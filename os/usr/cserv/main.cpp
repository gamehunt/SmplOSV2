/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <sys/syscall.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fb.h>
#include <kbd.h>
#include <vector>
#include <ctime>
#include <cserv/cserv.h>
#include <cserv/widgets/widget.h>

#include <kernel/memory/memory.h>

FILE* keyboard;
FILE* mouse;

CSProcess* active_process = 0;

int mx=0;
int my=0;

uint32_t clks = 0;

typedef struct{
	int32_t x_mov;
	int32_t y_mov;
	uint8_t buttons;
}mouse_packet_t;

int process_packet(){
	CSPacket* packet = CServer::S_LastPacket();
	while(packet){
		if(packet->GetType() == CS_TYPE_PROCESS){
				CSProcess* process = CSProcess::CreateProcess(BUFFER(packet,pid_t,0),0,0,BUFFER(packet,uint32_t,1),BUFFER(packet,uint32_t,2));
				CServer::S_AddProcess(process);
				if(!active_process){
					active_process = process;
				}
		}else if(packet->GetType() == CS_TYPE_ACTIVATE){
				for(CSProcess* proc : CServer::S_GetAllProcesses()){
					if(proc->GetPid() == ((int*)packet->GetBuffer())[0]){
						active_process = proc;
						break;
					}
				}	
		}else if(packet->GetType() == CS_TYPE_TERMINATE){
				//Here we should delete process with sent pid from processes
		}else if(packet->GetType() == CS_TYPE_KEY && active_process && !active_process->ApplyFilter(packet)){
				for(uint8_t i=0;i<128&&packet->GetBuffer()[i];i++){
					key_t* key = (key_t*)malloc(sizeof(key_t));
					kbd_key_event(key,packet->GetBuffer()[i]);
					if(key->key && key->state == KEY_ACTION_DOWN){
						char buffer[64];
						sprintf(buffer,"/proc/%d/stdin",active_process->GetPid());
						FILE* in = fopen(buffer,"w");
						if(in){
							fwrite(&key->key,1,1,in);
							fclose(in);
						}
					}
					free(key);
				}
		}else if(active_process && active_process->ApplyFilter(packet)){
				FILE* sock = CServer::OpenSocket(active_process->GetPid());
				if(sock){
					fwrite(packet,sizeof(CSPacket),1,sock);
					fclose(sock);
				}
		}
		delete packet;
		packet = CServer::S_LastPacket();
	}
	return 0;
}

void render_thread(){
	while(1){
		fb_rect(0,0,1024,768,0x00000000,true,0);
		for(CSProcess* proc : CServer::S_GetAllProcesses()){
			shmem_block_t* block = new shmem_block_t;
			sys_shmem_open(proc->GetPid(),0,block);
			uint32_t* shared = (uint32_t*)(SHARED_MEMORY_START + block->offset);
			//sys_echo("%d %d %d %d %d\n",proc->GetCanvasX(),proc->GetCanvasY(),proc->GetCanvasWidth(),proc->GetCanvasHeight(),block->offset);
			fb_inject_buffer(proc->GetCanvasX(),proc->GetCanvasY(),proc->GetCanvasWidth(),proc->GetCanvasHeight(),shared,0);
			sys_shmem_reset();
		}
		fb_rect(mx,my,4,4,0x0000FF00,true,0);
		fb_swapbuffers();
		sys_sleep(5);
	}
}

int main(int argc,char** argv){
	
	setpriority(0,0,1);
	
	if(CServer::Init("/dev/cserver")){
		sys_echo("[CSRV] Failed to initialize server!\n");
		return 1;
	}
	
	keyboard = fopen("/dev/kbd","r");
	mouse    = fopen("/dev/mouse","r");
	
	if(!keyboard){
		sys_echo("[CSRV] Failed to open input devices! (keyboard)\n");
		return 1;
	}
	
	if(!mouse){
		sys_echo("[CSRV] Failed to open input devices! (mouse)\n");
		return 1;
	}
	
	uint8_t buffer[3072];
	fread(buffer,1,128,keyboard);
	fread(buffer,1,3072,mouse);

	sys_echo("[CSRV] Setting up framebuffer...\n");

	FILE* fb = fopen("/dev/fb0","w");
	uint16_t args[] = {1024,768,0x20};
	if(fb){
		sys_ioctl(fb->fd,0x10,args);
	}else{
		sys_echo("[CSRV] Failed to initialize framebuffer\n");
		return 1;
	}
	
	if(fb_init("/dev/fb0",1024,768,1)){
		sys_echo("[CSRV] Failed to initialize framebuffer\n");
		return 1;
	}
	
	sys_send(getppid(),0); //send SIG_CHILD 
	
	sys_thread((uint32_t)render_thread);
	
	sys_echo("[CSRV] Started server\n");
	
	uint32_t node = 0;
	uint32_t* fds = new uint32_t[3]{keyboard->fd,mouse->fd,CServer::GetServerPipe()->fd};
	
	while(1){
		
		node = sys_fswait(fds,3);
		
		if(node == 0){
			CSPacket* pack = CSPacket::CreatePacket(CS_TYPE_KEY);
			fread(pack->GetBuffer(),1,128,keyboard);
			CServer::C_SendPacket(pack);
			
		}
		if(node == 1){
			 mouse_packet_t* packets = new mouse_packet_t[256];
			 int read = fread(packets,sizeof(mouse_packet_t),256,mouse);
			 rewind(mouse);
			 int pmx = mx;
			 int pmy = my;
			 for(int i=0;i<read;i++){
				 mx += packets[i].x_mov / 2; //TODO calculate proper values for division
				 my -= packets[i].y_mov / 2;
				 if(mx < 0){
					mx = 0;
				 }
				 if(my < 0){
					my = 0;
				 }
				 if(mx > 1023){
					mx = 1023;
				 }
				 if(my > 767){
					my = 767;
				 }
				 if(packets[i].buttons){
					 
					 clks++;
				 }
			 }
			 delete[] packets;
			//TODO send CS_TYPE_MOUSE to anywhere
		}
		process_packet();
	}
	
	return 0;
}
