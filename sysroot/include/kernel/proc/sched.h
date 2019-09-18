#pragma once

#include <stdint.h>
//#include <kernel/global.h>

typedef struct{
		uint32_t cr3;
		uint32_t eip;
		uint32_t esp;
		uint32_t ebp;
}context_t;

typedef struct{
	uint32_t pid;
	context_t* ctx;
}process_t;

void write_context(context_t* ctx);
void setup_context(context_t* ctx);
void scheduler_tick();

void init_sched();



