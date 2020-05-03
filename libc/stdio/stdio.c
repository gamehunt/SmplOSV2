#include <stdio.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <unistd.h>
#include <kernel/interrupts/syscalls.h>

FILE _stdout={
		.fd = 0,
		.offset =0,
		.eof = 0,
		.buffer = 0,
		.error = 0,
		.size = 0
};

FILE _stderr={
		.fd = 1,
		.offset =0,
		.eof = 0,
		.buffer = 0,
		.error = 0,
		.size = 0
};

FILE _stdin={
		.fd = 2,
		.offset =0,
		.eof = 0,
		.buffer = 0,
		.error = 0,
		.size = 0
};

FILE* stdout = &_stdout;
FILE* stdin = &_stdin;
FILE* stderr = &_stderr;

size_t fwrite(const void* ptr, size_t sz, size_t block_count, FILE* f){
	if(f->eof){
	//	sys_echo("EOF\n");
		return 0;
	}
	uint32_t real_size = sz * block_count;
	uint32_t writen = sys_write(f->fd,f->offset,real_size,ptr);
	f->offset += writen;
	if(f->size && f->offset >= f->size){
		f->eof = 1;
	}
	return writen / sz;
}

size_t fread(void* ptr, size_t sz, size_t block_count, FILE* f){
	if(f->eof){
		return 0;
	}
	uint32_t real_size = sz * block_count;
	uint32_t readen = sys_read(f->fd,f->offset,real_size,ptr);
	f->offset += readen;
	if(f->size && f->offset >= f->size){
		f->eof = 1;
	}
	return  readen / sz;
}

//TODO more modes
FILE* fopen(const char* name,const char* mode){
	FILE* f = malloc(sizeof(FILE));
	memset(f,0,sizeof(FILE));
	//sys_echo("%d\n",sizeof(stat_t));
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
		stat_t* stat = malloc(sizeof(stat_t));
		sys_stat(fd,stat);
		f->size = stat->st_size;
		free(stat);
		return f;
	} 
}

int fclose(FILE* f){
	sys_close(f->fd);
	free(f);
	return 0;
}

FILE *fdopen(int fildes, const char *mode){
	
}
