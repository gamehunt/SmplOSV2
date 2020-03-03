/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <string.h>

int strlen(const char* str){
	int size = 0;
	while(str[size] != '\0'){
		size++;
	}
	return size;
}
