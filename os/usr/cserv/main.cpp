/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <sys/syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fb.h>
#include <kbd.h>
#include <vector>
#include <cserv/cserv.h>
#include <cserv/widgets/widget.h>
#include <cserv/widgets/debug.h>

FILE* keyboard;


CSProcess* active_process = 0;

CSWidget* widget_from_id(int id){
	switch(id){
		case 0:
			return new CSDebugWidget(0,0);
	}
	return 0;
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
				if(proc){
						CSWidget* wid = widget_from_id(((uint32_t*)packet->GetBuffer())[1]);
						if(wid){
							proc->AddWidget(wid);
						}
				}
		}else if(packet->GetType() == CS_TYPE_KEY && active_process && !active_process->ApplyFilter(packet)){
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
	
	if(CServer::Init("/dev/cserver")){
		sys_echo("Failed to initialize server!",0);
	}
	
	keyboard = fopen("/dev/kbd","r");

	sys_echo("Setting up framebuffer...",0);

	FILE* fb = fopen("/dev/fb0","w");
	uint16_t args[] = {1024,768,0x20};
	if(fb){
		sys_ioctl(fb->fd,0x10,args);
	}else{
		return 1;
	}
	
	if(fb_init("/dev/fb0",1024,768)){
		sys_echo("Failed to initialize framebuffer",0);
		return 1;
	}
	
	uint32_t node = 0;
	
	sys_echo("Started server",0);
	
	sys_send(getppid(),0); //send SIG_CHILD 
	
	while(1){
		node = sys_fswait(new uint32_t[2]{keyboard->fd,CServer::GetServerPipe()->fd},2);
		if(node == 0){
			CSPacket* pack = CSPacket::CreatePacket(CS_TYPE_KEY);
			fread(pack->GetBuffer(),1,128,keyboard);
			CServer::C_SendPacket(pack);
		}
		process_packet();
		sys_ioctl(fb->fd,0x10,args); //This hack will clear all memory, TODO: make something less stupid to do that
		CServer::S_Tick();
	}
	
	return 0;
}
