#include <stdio.h>
#include <sys/syscall.h>
#include <kernel/interrupts/syscalls.h>

FILE _stdout={
		.fd = 0,
		.offset =0
};

FILE _stderr={
		.fd = 1,
		.offset =0
};

FILE _stdin={
		.fd = 2,
		.offset =0
};

FILE* stdout = &_stdout;
FILE* stdin = &_stdin;
FILE* stderr = &_stderr;

//TODO SEEK SET

size_t fwrite(const void* ptr, size_t sz, size_t block_count, FILE* f){
	uint32_t real_size = sz * block_count;
	return sys_write(f->fd,f->offset,real_size,ptr) / sz;
}

size_t fread(void* ptr, size_t sz, size_t block_count, FILE* f){
	uint32_t real_size = sz * block_count;
	uint32_t readen = sys_read(f->fd,f->offset,real_size,ptr);
	return  readen / sz;
}

//TODO more modes
FILE* fopen(const char* name,const char* mode){
	FILE* f = malloc(sizeof(FILE));
	uint8_t flags = 0;
	if(!strcmp(mode,"r")){
		flags |= F_READ;
	}
	if(!strcmp(mode,"w")){
		flags |= F_WRITE;
	}
	if(!strcmp(mode,"r+")){
		flags |= F_READ;
		flags |= F_WRITE;
		flags |= F_CREATE;
	}
	int fd = sys_open(name,flags);
	if(fd < 0){
		free(f);
		return 0;
	}else{
		f->fd = (uint32_t)fd;
	} 
	return f;
}

int fclose(FILE* f){
	sys_close(f->fd);
	free(f);
	return 0;
}

FILE *fdopen(int fildes, const char *mode){
	
}
