/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdio.h>
#include <sys/syscall.h>

int main(int argc,char** argv){

	sys_pwreq(SYS_PWREQ_SHUTDOWN);
	
	return 0;
}
