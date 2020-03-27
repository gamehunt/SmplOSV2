#include <stdint.h>
#include <kernel/interrupts/syscalls.h>

#if !defined(__smplos_libk) && !defined(__smplos_kernel) 

extern uint32_t sys_call(uint32_t n,uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e);

static inline uint32_t sys_echo(char* str,uint32_t opt){
	return sys_call(SYS_ECHO,(uint32_t)str,opt,0,0,0);
}

static inline uint32_t sys_read(uint32_t fd,uint64_t offset, uint32_t size,uint8_t* buffer){
	uint32_t offs_high = (uint32_t)(offset >> 32);
	uint32_t offs_low = (uint32_t)(offset);
	return sys_call(SYS_READ,fd,offs_high,offs_low,size,(uint32_t)buffer);
}

static inline uint32_t sys_write(uint32_t fd,uint64_t offset,uint32_t size,uint8_t* buffer){
	uint32_t offs_high = (uint32_t)(offset >> 32);
	uint32_t offs_low = (uint32_t)(offset);
	return sys_call(SYS_WRITE,fd,offs_high,offs_low,size,(uint32_t)buffer);
}

static inline uint32_t sys_open(char* path){
	return sys_call(SYS_OPEN,(uint32_t)path,0,0,0,0);
}


static inline uint32_t sys_close(uint32_t fd){
	return sys_call(SYS_CLOSE,fd,0,0,0,0);
}

static inline uint32_t sys_readdir(uint32_t fd){
	return sys_call(SYS_READDIR,fd,0,0,0,0);
}

static inline uint32_t sys_exec(char* path,uint32_t argc,char** argv,char** envp){
	return sys_call(SYS_EXEC,(uint32_t)path,argc,(uint32_t)argv,(uint32_t)envp,0);
}

static inline uint32_t sys_clone(){
	return sys_call(SYS_CLONE,0,0,0,0,0);
}

static inline uint32_t sys_ioctl(uint32_t fd,uint32_t req,void* argp){
	return sys_call(SYS_IOCTL,fd,req,(uint32_t)argp,0,0);
}

static inline uint32_t sys_exit(uint32_t code){
	return sys_call(SYS_EXIT,code,0,0,0,0);
}


static inline uint32_t sys_fswait(uint32_t* fds,uint32_t cnt){
	return sys_call(SYS_FSWAIT,(uint32_t)fds,cnt,0,0,0);
}

static inline uint32_t sys_yield(){
	return sys_call(SYS_YIELD,0,0,0,0,0);
}

static inline uint32_t sys_sbrk(uint32_t size){
	return sys_call(SYS_SBRK,size,0,0,0,0);
}

#endif
