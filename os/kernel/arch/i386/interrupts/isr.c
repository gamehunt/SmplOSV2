/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/interrupts/isr.h>
#include <kernel/io/io.h>
#include <kernel/misc/panic.h>
#include <kernel/proc/proc.h>
static irq_handler_t isr_handlers[256] = {0};

const char *exc_m[] = {
	"Division by zero",				
	"Debug",
	"Non-maskable interrupt",
	"Breakpoint",
	"Detected overflow",
	"Out-of-bounds",				
	"Invalid opcode",
	"No coprocessor",
	"Double fault",
	"Coprocessor segment overrun",
	"Bad TSS",						/* 10 */
	"Segment not present",
	"Stack fault",
	"General protection fault",
	"Page fault",
	"Unknown interrupt",			
	"Coprocessor fault",
	"Alignment check",
	"Machine check",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};

void isr_set_handler(uint16_t isr,irq_handler_t handler){
	isr_handlers[isr] = handler;
}

void isr_unset_handler(uint16_t isr){
	isr_handlers[isr] = 0;
}

void fault_handler(regs_t r){
	lock_interrupts();
	irq_handler_t handler = isr_handlers[r->int_no];
	if(handler){
		handler(r);
	}else if(get_current_process() && get_current_process()->pid != 1){
		kinfo("Process %s caused exception: %s\n",get_current_process()->name,exc_m[r->int_no]);
		proc_exit(get_current_process());
	}else{
		mem_check();
		crash_info_t crash;	
		crash.regs = r;	
		crash.description = exc_m[r->int_no];
		char message[128];
		sprintf(message,"kpanic() invoked via unhandled isr\n[E] Error code: %p\n",r->err_code);
		crash.extra_info = message;
		kpanic(crash);
	}
	unlock_interrupts();
}

volatile int interrupt_locker = 0;

void lock_interrupts(){
	interrupt_locker = 1;
}
void unlock_interrupts(){
	interrupt_locker = 0;
}

void disable_interrupts(){
	if(!interrupt_locker){
		asm("cli");
	}
}
void enable_interrupts(){
	if(!interrupt_locker){
		asm("sti");
	}
}
