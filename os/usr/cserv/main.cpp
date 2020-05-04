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
#include <cserv/cserv.h>
#include <cserv/widgets/widget.h>
#include <cserv/widgets/debug.h>
#include <cserv/widgets/text_screen.h>

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

CSWidget* widget_from_id(int id){
	switch(id){
		case 0:
			return new CSDebugWidget(0,0);
		case 1:
			return new CSTextScreenWidget(0,0);
	}
	return 0;
}

void render(){
	while(1){
		fb_rect(0,0,1024,768,0x00000000,1);
		CServer::S_Tick();
		/* --mouse debug code
		char num[32];
		memset(num,0,32);
		sprintf(num,"X: %d",mx);
		for(int i=0;i<strlen(num);i++){
			fb_char(num[i],10*(i+1),16,0x00FF0000,0x00000000);
		}
		memset(num,0,32);
		sprintf(num,"Y: %d",my);
		for(int i=0;i<strlen(num);i++){
			fb_char(num[i],10*(i+1),33,0x00FF0000,0x00000000);
		}
		memset(num,0,32);
		sprintf(num,"Clicks: %d",clks);
		for(int i=0;i<strlen(num);i++){
			fb_char(num[i],10*(i+1),70,0x00FF0000,0x00000000);
		}
		* */
		//fb_char('A',mx,my,0x0000FF00,0x00000000);
		fb_swapbuffers();
		
		sys_sleep(5); //We need this to allow lower priority process schedule
	}
}

int process_packet(){
	CSPacket* packet = CServer::S_LastPacket();
	while(packet){
		if(packet->GetType() == CS_TYPE_PROCESS){
				CServer::S_AddProcess(((pid_t*)packet->GetBuffer())[0]);
				if(!active_process){
					active_process = CServer::S_GetAllProcesses()[0];
				}
		}else if(packet->GetType() == CS_TYPE_ACTIVATE){
				for(CSProcess* proc : CServer::S_GetAllProcesses()){
					if(proc->GetPid() == ((int*)packet->GetBuffer())[0]){
						active_process = proc;
					}
				}	
		}else if(packet->GetType() == CS_TYPE_TERMINATE){
				//Here we should delete process with sent pid from processes
		}else if(packet->GetType() == CS_TYPE_WIDGET){
				pid_t pid = ((pid_t*)packet->GetBuffer())[0];
				CSProcess* proc = CServer::S_GetProcess(pid);
				uint32_t type = ((uint32_t*)packet->GetBuffer())[1];
				if(proc){
						
						if(type == WIDGET_PACK_ADD){
							CSWidget* wid = widget_from_id(((uint32_t*)packet->GetBuffer())[2]);
							if(wid){
								proc->AddWidget(wid);
							}
						}else if(type == WIDGET_PACK_UPD){
							CSWidget* wid = proc->GetWidget(((uint32_t*)packet->GetBuffer())[2]);
							if(wid){
								void* buffer = (void*)(&(((uint32_t*)packet->GetBuffer())[3]));
								if(!packet->GetFlags() & CS_PACK_FLAG_EXTENDED){
									wid->Update(buffer);
								}else{
									uint32_t bid = ((uint32_t*)buffer)[0];
									shmem_block_t* own_b = (shmem_block_t*)malloc(sizeof(shmem_block_t));
									sys_shmem_open(pid,bid,own_b);
									void* shared_buffer = (void*)(SHARED_MEMORY_START + own_b->offset);
									wid->Update(shared_buffer);
									sys_shmem_reset();
								}
							}
						}else if(type == WIDGET_PACK_MOV){
							CSWidget* wid = proc->GetWidget(((uint32_t*)packet->GetBuffer())[2]);
							if(wid){
								wid->SetX(((uint32_t*)packet->GetBuffer())[3]);
								wid->SetY(((uint32_t*)packet->GetBuffer())[4]);
							}
						}else if(type == WIDGET_PACK_RES){
							CSWidget* wid = proc->GetWidget(((uint32_t*)packet->GetBuffer())[2]);
							if(wid){
								wid->SetSizeX(((uint32_t*)packet->GetBuffer())[3]);
								wid->SetSizeY(((uint32_t*)packet->GetBuffer())[4]);
							}
							
						}
				}
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

int main(int argc,char** argv){
	
	setpriority(0,0,1);
	
	if(CServer::Init("/dev/cserver")){
		sys_echo("[CSRV] Failed to initialize server!\n");
		return 1;
	}
	
	keyboard = fopen("/dev/kbd","r");
	mouse    = fopen("/dev/mouse","r");
	
	//if(!keyboard || !mouse){
	//	sys_echo("[CSRV] Failed to open input devices!\n");
	//	return 1;
	//}
	
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
	sys_echo("[CSRV] Started server\n");
	
	//while(1);
	
	
	sys_thread((uint32_t)&render);
	
	uint32_t node = 0;
	uint32_t* fds = new uint32_t[3]{keyboard->fd,mouse->fd,CServer::GetServerPipe()->fd};
	
	while(1){
		
		node = sys_fswait(fds,3);
		
		if(node == 0){
			CSPacket* pack = CSPacket::CreatePacket(CS_TYPE_KEY);
			//sys_echo("%d\n",sizeof(CSPacket));
			
			fread(pack->GetBuffer(),1,128,keyboard);
			//rewind(keyboard);
			CServer::C_SendPacket(pack);
			
		}
		if(node == 1){
			 mouse_packet_t* packets = new mouse_packet_t[256];
			 int read = fread(packets,sizeof(mouse_packet_t),256,mouse);
			 rewind(mouse);
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
		//mem_check();
		//while(1);
	}
	
	return 0;
}
