#include<kernel/interrupts/isr.h>
#include<kernel/io/io.h>
#include <kernel/misc/panic.h>
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
	asm("cli");
	//while(1);
	irq_handler_t handler = isr_handlers[r->int_no];
	if(handler){
		handler(r);
	}else{
		crash_info_t crash;	
		crash.regs = r;	
		crash.description = exc_m[r->int_no];
		crash.extra_info = "kpanic() invoked via unhandled isr";
		kpanic(crash);
	}
	asm("sti");
}
