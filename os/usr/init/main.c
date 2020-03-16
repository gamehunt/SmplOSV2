/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdint.h>
#include <sys/syscall.h>

void _start(){
	const char* str  = "SYSCALL 0";
	const char* str1  = "SYSCALL 1";
	
	sys_call(0,(uint32_t)str,0,0,0,0);
	sys_call(1,(uint32_t)str1,0,0,0,0);
	sys_call(0,(uint32_t)str,0,0,0,0);
	while(1);
}
