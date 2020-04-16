#include <stdio.h>

long ftell(FILE *stream){
	return stream->offset;
}
