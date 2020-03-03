/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include<kernel/interrupts/irq.h>
#include<kernel/interrupts/pic.h>
#include<kernel/io/io.h>
static irq_handler_t irq_handlers[16] = {0};

void irq_set_handler(uint8_t irq,irq_handler_t handler){
	irq_handlers[irq] = handler;
}

void irq_unset_handler(uint8_t irq){
	irq_handlers[irq] = 0;
}
void irq_end(uint8_t int_no){
	//printf("%dE\n",int_no);
	if (int_no >= 8) {
		outb(0xA0, 0x20);
	}
	outb(0x20, 0x20);
}

void irq_handler(regs_t r){
	asm("cli");
	irq_handler_t handler;
	if(r -> int_no > 47 || r->int_no < 32){
		handler = 0;
	}else{
		handler = irq_handlers[r->int_no - 32];
	}
	if(handler){
		handler(r);
	}else{
		irq_end(r->int_no - 32);
	}
	asm("sti");
}
