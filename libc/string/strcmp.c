#include <string.h>

uint32_t strcmp(const char *X, const char *Y)
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