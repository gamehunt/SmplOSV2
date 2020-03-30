#pragma once

#include <stdint.h>
#include <string.h>

static char* canonize_absolute(char* path){
	uint32_t size = strlen(path);	
	if(path[0]!='/'){
		size++;
	}
	if(path[strlen(path)-1] != '/'){
		size++;
	}
	char* npath = kmalloc(size+2);
	memset(npath,0,size+2);
	if(path[0]!='/'){
		npath[0]='/';
	}
	strcat(npath,path);
	if(path[strlen(path)-1] != '/'){
		strcat(npath,"/");
	}
	return npath;
}

static uint32_t path_size(char* path){
	uint32_t blocks = 0 ;
	for(int i=0;i<strlen(path);i++){
		if(path[i] == '/'){
			blocks++;
		}
	}
	return blocks-1;
}

static char* path_block(char* path,uint32_t block){
	uint32_t blocks = 0 ;
	int a=-1;
	int b=-1;
	for(int i=0;i<strlen(path);i++){
		if(blocks==block && path[i] == '/'){
			if(a < 0){
				a = i+1;
			}else{
				b = i;
				break;
			}
		}else if(path[i] == '/'){
			blocks++;
		}
	}
	if(a > 0 && b > 0){
		return substr(path,a,b);
	}
	return path;
}
