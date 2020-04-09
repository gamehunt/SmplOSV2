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

FILE* keyboard;


CSProcess* active_process = 0;

int process_packet(){
	CSPacket* packet = CServer::S_LastPacket();
	FILE* sock;
	if(packet){
		switch(packet->GetType()){
			case CS_TYPE_PROCESS:
				CServer::S_AddProcess(((pid_t*)packet->GetBuffer())[0]);
				if(!active_process){
					active_process = CServer::S_GetAllProcesses()[0];
				}
				sys_echo("Added process to cserver\n",0);
			break;
			case CS_TYPE_ACTIVATE:
				for(CSProcess* proc : CServer::S_GetAllProcesses()){
					if(proc->GetPid() == ((int*)packet->GetBuffer())[0]){
						active_process = proc;
					}
				}	
			break;
			default:
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
		return 1;
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

	gdi_init("/dev/fb0",1024,768);

	execv("/usr/bin/term.smp",0);

	while(1){
		sys_fswait(new uint32_t[2]{keyboard->fd,CServer::GetServerPipe()->fd},2);
		process_packet();
		CServer::S_Tick();
	}
	
	return 0;
}
