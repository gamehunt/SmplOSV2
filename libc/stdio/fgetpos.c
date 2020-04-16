#include <stdio.h>

int fgetpos(FILE *stream, fpos_t *pos){
	*pos = stream->offset;
	return 0;
}
