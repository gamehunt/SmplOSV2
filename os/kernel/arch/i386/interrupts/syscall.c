/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/interrupts/syscalls.h>
#include <kernel/interrupts/isr.h>

void syscall_handler(regs_t r){
	kinfo("SYSCALL: %a\n",r->eax);
}

void init_syscalls(){
	isr_set_handler(127,&syscall_handler);
}
