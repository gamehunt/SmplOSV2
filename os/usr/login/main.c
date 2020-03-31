/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdio.h>
#include <dirent.h>


//Here will be login system
int main(int argc,char** argv){
	
//lol
while(1){
	printf("Enter login:");
	char login[32];
	fgets(login,32,stdin);
	if(strcmp(login,"root")){
		printf("No such user\n");
	}else{
		char password[64];
		printf("Enter password:");
		fgets(password,64,stdin);
		if(strcmp(password,"123456")){
			printf("Invalid password\n");
		}else{
			printf("You logged in as a root\n");
			break;
		}
	}
}
	return 0;
}
