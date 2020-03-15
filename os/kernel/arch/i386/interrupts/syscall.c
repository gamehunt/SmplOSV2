/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/interrupts/syscalls.h>
#include <kernel/interrupts/isr.h>

#define MAX_SYSCALL 128

typedef uint32_t(* syscall_t)(uint32_t a,uint32_t b,uint32_t c,uint32_t e,uint32_t f);

static syscall_t syscalls[MAX_SYSCALL];

void syscall_handler(regs_t r){
	if(r->eax > MAX_SYSCALL){
		kerr("Invalid syscall: %a\n",r->eax);
		return;
	}
	syscall_t sysc = syscalls[r->eax];
	if(sysc){
		uint32_t ret = sysc(r->ebx,r->ecx,r->edx,r->esi,r->edi);
		r->eax = ret;
	}else{
		kerr("Syscall %a has null handler\n",r->eax);
	}
}

void register_syscall(uint16_t id,syscall_t handler){
	syscalls[id] = handler;
}

uint32_t sys_echo(uint32_t str,uint32_t _,uint32_t __,uint32_t ___,uint32_t _____){
	kinfo("[SYS_ECHO] %a: %s\n",str,(char*)str);
	return 0;
}

void init_syscalls(){
	isr_set_handler(127,&syscall_handler);
	memset(syscalls,0,sizeof(syscall_t)*MAX_SYSCALL);
	
	register_syscall(0,&sys_echo);
}
