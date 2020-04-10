/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <cserv/cserv.h>
#include <gdi.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <gdi.h>
#include <kbd.h>

FILE* keyboard;


CSProcess* active_process = 0;

int process_packet(){
	CSPacket* packet = CServer::S_LastPacket();
	FILE* sock;
	while(packet){
		switch(packet->GetType()){
			case CS_TYPE_PROCESS:
				CServer::S_AddProcess(((pid_t*)packet->GetBuffer())[0]);
				if(!active_process){
					active_process = CServer::S_GetAllProcesses()[0];
				}
			break;
			case CS_TYPE_ACTIVATE:
				for(CSProcess* proc : CServer::S_GetAllProcesses()){
					if(proc->GetPid() == ((int*)packet->GetBuffer())[0]){
						active_process = proc;
					}
				}	
			break;
			case CS_TYPE_KEY:
				if(!active_process){
					break;
				}
				if(!active_process->ApplyFilter(packet)){
					for(uint8_t i=0;i<128&&packet->GetBuffer()[i];i++){
						key_t* key = (key_t*)malloc(sizeof(key));
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
					break;
				}
				//Fallback to default
			default:
				if(!active_process){
					break;
				}
				if(!active_process->ApplyFilter(packet)){
					break;
				}
				sock = CServer::OpenSocket(active_process->GetPid());
				if(sock){
					fwrite(packet,sizeof(CSPacket),1,sock);
					fclose(sock);
				}
			break;
		}
		delete packet;
		packet = CServer::S_LastPacket();
	}
	return 0;
}

int main(int argc,char** argv){
	
	if(CServer::Init("/dev/cserver")){
		sys_echo("Failed to initialize server!",0);
	}
	
	keyboard = fopen("/dev/kbd","r");
	
	char inputbuffer[256];

	sys_echo("Starting server...",0);

	FILE* fb = fopen("/dev/fb0","w");
	if(fb){
		uint16_t args[] = {1024,768,0x20};
		sys_ioctl(fb->fd,0x10,args);
		fclose(fb);
	}	

	execv("/usr/bin/term.smp",0);

	sys_yield();
	
	uint32_t node = 1;
	
	while(1){
		if(node == 0){
			CSPacket* pack = CSPacket::CreatePacket(CS_TYPE_KEY);
			fread(pack->GetBuffer(),1,128,keyboard);
			CServer::C_SendPacket(pack);
		}
		process_packet();
		CServer::S_Tick();
		node = sys_fswait(new uint32_t[2]{keyboard->fd,CServer::GetServerPipe()->fd},2);
	}
	
	return 0;
}
