/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <string.h>

int strcmp(const char *X, const char *Y)
{
	while(*X)
	{
		// if characters differ or end of second string is reached
		if (*X != *Y)
			break;

		// move to next pair of characters
		X++;
		Y++;
	}

	// return the ASCII difference after converting char* to unsigned char*
	return *(const unsigned char*)X - *(const unsigned char*)Y;
}
