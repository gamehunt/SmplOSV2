/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

int main(int argc,char** argv){

	if(!argc){
		printf("Usage: echo [text]");
	}else{
		for(int i=0;i<argc;i++){
			printf("%s ",argv[i]);
		}
	}
	//write(1,0,0);
	return 0;
}
