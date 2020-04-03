#include <ctype.h>

uint8_t isprint(int c){
	return c >= 0x20 && c<= 0x7E;
}
