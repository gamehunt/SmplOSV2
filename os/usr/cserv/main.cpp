/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <cserv/cserv.h>
#include <gdi.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/syscall.h>

FILE* server_pipe;

int recv_packet(){
	int res = 0;
	//sys_echo("Allocation size: ",sizeof(cspacket<int>));
	cspacket<int> packet = (cspacket<int>*)malloc(sizeof(cspacket<int>));
	if(fread(packet,sizeof(cspacket<int>),1,server_pipe)){
		switch(packet->get_type()){
			case CS_TYPE_PROCESS:
				
			res =  1;	
			break;
		}
		
		
	}
	free(packet);
	return res;
}

int main(int argc,char** argv){
	
	
	FILE* master_pipe = fopen("/dev/pipe","");
	
	sys_pipe("/dev/cserver",4096);
	
	server_pipe = fopen("/dev/cserver","r+");
	
	while(1){
		recv_packet();
	}
	
	return 0;
}
