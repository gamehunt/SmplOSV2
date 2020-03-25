#include <stdint.h>
#include <kernel/interrupts/syscalls.h>

extern uint32_t sys_call(uint32_t n,uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e);

uint32_t sys_read(uint32_t fd,uint32_t offset, uint32_t size,uint32_t buffer){
	
}

uint32_t sys_write(uint32_t fd,uint32_t offset,uint32_t size,uint32_t buffer){
	
}

uint32_t sys_open(char* path){

}


uint32_t sys_close(uint32_t fd){

}

uint32_t sys_readdir(uint32_t fd){

}

uint32_t sys_exec(char* path,uint32_t argc,uint32_t argv){

}

uint32_t sys_clone(){
	
}

uint32_t sys_ioctl(uint32_t fd,uint32_t req,void* argp){

}

uint32_t sys_exit(uint32_t code,uint32_t _,uint32_t __,uint32_t ___,uint32_t _____){
	
}


uint32_t sys_fswait(uint32_t fds,uint32_t cnt,uint32_t __,uint32_t ___,uint32_t _____){

}

uint32_t sys_yield(uint32_t _,uint32_t __,uint32_t ___,uint32_t ____,uint32_t _____){

}

static inline uint32_t sys_sbrk(uint32_t size){
	return sys_call(SYS_SBRK,size,0,0,0,0);
}
