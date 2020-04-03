/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <string.h>

int strncat( char * string1, const char * string2, size_t num ){
	char* ptr = string1 + strlen(string1);
	while(*string2 != '\0' && num--){
		*ptr++ = *string2++;
	}
	*ptr = '\0';
	return string1;
}
