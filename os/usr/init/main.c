/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdint.h>
#include <kernel/interrupts/syscalls.h>
#include <kernel/proc/proc.h>
#include <sys/syscall.h>
#include <kernel/fs/vfs.h>

volatile uint8_t cserv_exec = 0;

int sig_child(){
	cserv_exec = 1;
	sys_echo("INIT received SIGCHILD\n");
	sys_sigexit();
}

int main(){
	//*((uint8_t*)0xFFFFFFFF) = 0;
	sys_echo("[INI] Init start\n");
	
	setenv("PATH","/usr/bin",1);
	
	sys_signal(SIG_CHILD,sig_child);
	//TODO this should be readen from some file
	execv("/usr/bin/cserv.smp",0);
	while(!cserv_exec); //Wait unitl cserver send us SIGCHILD
	execv("/usr/bin/term.smp",0);
	
	while(1);
	return 0;
}
