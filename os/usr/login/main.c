/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdio.h>
#include <dirent.h>


//Here will be login system
int main(int argc,char** argv){

	printf("You logged in as a root\n");
	execv("/usr/bin/shell.smp",0);
	return 0;
}
