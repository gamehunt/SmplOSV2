#pragma once 

#include<kernel/io/io.h>
#include<kernel/interrupts/pic.h>
#include<kernel/global.h>

typedef void(* irq_handler_t) (struct registers *);
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

void irq_end(uint8_t irq);

void irq_set_handler(uint8_t irq,irq_handler_t handler);
void irq_unset_handler(uint8_t irq);

void irq_handler(regs_t r);
