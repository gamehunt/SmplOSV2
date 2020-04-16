#include <stdio.h>

void setbuf(FILE* s, char* buf){
	s->buffer = buf;
}
