/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdint.h>
#include <sys/syscall.h>
#include <kernel/fs/vfs.h>

int main(){
	//while(1);
	char* out = "Launched shell";
	char* path = "/dev/kbd";
	char* msg = "Keycode received";
	sys_call(0,out,0,0,0,0);
	fs_node_t* kbd = sys_call(3,path,0,0,0,0);
	while(1){
		sys_call(8,&kbd,1,0,0,0);
		sys_call(0,msg,0,0,0,0);
	}
	sys_call(7,0,0,0,0,0);
	while(1);
}
