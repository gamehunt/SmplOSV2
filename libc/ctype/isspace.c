#include <ctype.h>

uint8_t isspace(int c){
	return c==' ' || c == '\n' || c == '\t' || c == '\v' || c == '\f' || c=='\r';
}
