#include <stdio.h>

void rewind(FILE *stream){
	stream->offset = 0;
	stream->eof = 0;
	stream->error = 0;
}
