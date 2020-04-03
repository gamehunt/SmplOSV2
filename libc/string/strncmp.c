/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <string.h>

int strncmp(const char *str1, const char *str2,size_t n)
{
	const char* end = str1+n;
    int result = 0;
    for(;result == 0 && str1 != end && (*str1 || *str2); result = *(str1++)-*(str2++));
    return result;
}
