/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <cserv/cserv.h>
#include <gdi.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/syscall.h>

#include <vector>

FILE* keyboard;


CSProcess* active_process = 0;

int process_packet(){
	CSPacket* packet = CServer::__s_LastPacket();
	
	if(packet){
		switch(packet->GetType()){
			case CS_TYPE_PROCESS:
				CServer::__s_AddProcess(((pid_t*)packet->GetBuffer())[0]);
				if(!active_process){
					active_process = CServer::__s_GetAllProcesses()[0];
				}
			break;
			case CS_TYPE_ACTIVATE:
				for(CSProcess* proc : CServer::__s_GetAllProcesses()){
					if(proc->GetPid() == ((int*)packet->GetBuffer())[0]){
						active_process = proc;
					}
				}	
			break;
			default:
				active_process->AddPacket(packet);
			break;
		}
		delete packet;
		return 1;
	}
	
	return 0;
}

int main(int argc,char** argv){
	
	CServer::Init("/dev/cserver");

	keyboard = fopen("/dev/kbd","r");
	
	char inputbuffer[256];

	while(1){
		sys_fswait(new uint32_t[2]{keyboard->fd,CServer::GetPipe()->fd},2);
		process_packet();
		CServer::__s_Tick();
	}
	
	return 0;
}
