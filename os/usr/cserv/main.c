/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <gdi.h>
#include <stdio.h>
#include <sys/syscall.h>

//TODO, first I need more power (libstdc++)

int main(int argc,char** argv){

	
	
	char* arg[] = {"/usr/bin/shell.smp",0};
	
	execv("/usr/bin/term.smp",arg);
	
	while(1){
		
		
	}
	
	return 0;
}
