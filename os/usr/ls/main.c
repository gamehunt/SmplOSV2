/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdio.h>
#include <dirent.h>

int main(int argc,char** argv){

	char buffer[256];
	if(!getcwd(buffer,256)){
		printf("Failed to get working directory\n");
		return 1;
	}
	DIR* d = opendir(buffer);
    if(!d){
		printf("Failed to open working directory\n");
		return 1;
	}
	struct dirent* file = 0;
	while((file = readdir(d))){
		printf("%s ",file->name);
		free(file);
	}
	closedir(d);
	return 0;
}
