/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <string.h>

int strncpy( char * destination, const char * source, size_t num ){
	if (destination == NULL)
		return NULL;

	char* ptr = destination;

	while (*source && num--)
	{
		*destination = *source;
		destination++;
		source++;
	}
	*destination = '\0';
	return ptr;
}
