/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once

#include <kernel/global.h>
#include <kernel/interrupts/irq.h>

#define PIT_A 0x40
#define PIT_B 0x41
#define PIT_C 0x42
#define PIT_CONTROL 0x43

#define PIT_MASK 0xFF
#define PIT_SCALE 1193180
#define PIT_SET 0x34

#define TIMER_IRQ 0

#define MAX_PIT_LISTENERS 65536

typedef struct{
	irq_handler_t handler;
	uint32_t time; //call every 'time' tick >= 0
}pit_listener_t;

uint32_t pit_add_listener(pit_listener_t* l);
void pit_remove_listener(uint32_t l);

void pit_phase(int hz);

void pit_tick(regs_t r);

void  init_pit();

uint32_t pit_system_ticks();
