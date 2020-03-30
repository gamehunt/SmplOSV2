/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdio.h>

int main(int argc,char** argv){

	if(!argc){
		printf("Usage: echo [text]");
	}else{
		for(int i=0;i<argc;i++){
			printf("%s ",argv[i]);
		}
	}
	return 0;
}
