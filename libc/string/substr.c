/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <string.h>

char* substr(const char *src, int m, int n)
{
	
	// get length of the destination string
	int len = n - m;
//kinfo("%d\n",sizeof(char) * (len + 1));
	// allocate (len + 1) chars for destination (+1 for extra null character)
	char *dest = (char*)malloc(sizeof(char) * (len + 1));
//kinfo("2\n");
	// extracts characters between m'th and n'th index from source string
	// and copy them into the destination string
	for (int i = m; i < n && (*src != '\0'); i++)
	{
		*dest = *(src + i);
		dest++;
	}
	//kinfo("3\n");

	// null-terminate the destination string
	*dest = '\0';
//kinfo("4\n");
	// return the destination string
	return dest - len;
}
