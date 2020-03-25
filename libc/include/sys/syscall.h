#include <stdint.h>
#include <kernel/interrupts/syscalls.h>

extern uint32_t sys_call(uint32_t n,uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e);

static inline uint32_t sys_sbrk(uint32_t size){
	return sys_call(SYS_SBRK,size,0,0,0,0);
}
