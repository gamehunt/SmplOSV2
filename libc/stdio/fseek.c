#include <stdio.h>


int fseek (FILE *stream, long offset, int origin){
	stream->ungetc = 0;
	switch(origin){
		case SEEK_CUR:
			if(stream->offset + offset > stream->size){
				return 1;
			}
			stream->offset += offset;
		break;
		case SEEK_SET:
			if(offset > stream->size){
				return 1;
			}
			stream->offset = offset;
		break;
		case SEEK_END:
			if(offset > stream->size){
				return 1;
			}
			stream->offset = stream->size - offset;
		break;
		default:
			return 1;
	}
	if(stream->offset >= stream->size){
		stream->eof = 1;
	}else{
		stream->eof = 0;
	}
	return 0;
}
