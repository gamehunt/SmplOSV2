#include <stdio.h>

int fsetpos(FILE *stream, const fpos_t *pos){
	stream->offset = *pos;
	return 0;
}
