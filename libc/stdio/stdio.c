#include <stdio.h>
#include <sys/syscall.h>

FILE _stdout={
		.fd = 0
};

FILE _stderr={
		.fd = 1
};

FILE _stdin={
		.fd = 2
};

FILE* stdout = &_stdout;
FILE* stdin = &_stdin;
FILE* stderr = &_stderr;

size_t fwrite(const void* ptr, size_t sz, size_t block_size, FILE* f){
	uint32_t real_size = sz * block_size;
	return sys_write(f->fd,0,block_size,f) / sz;
}

size_t fread(void* ptr, size_t sz, size_t block_size, FILE* f){
	uint32_t real_size = sz * block_size;
	return sys_read(f->fd,0,block_size,f) / sz;
}

FILE* fopen(const char* name,const char* mode){
	FILE* f = malloc(sizeof(FILE));
	f->fd = sys_open(name);
	return f;
}

int fclose(FILE* f){
	sys_close(f->fd);
	free(f);
	return 0;
}
