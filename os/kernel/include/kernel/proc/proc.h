/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/fs/vfs.h>

#include <kernel/global.h>

#define MAX_PROCESSES 256

#define PROC_INVALID 0
#define PROC_CREATED 1
#define PROC_RUN 2
#define PROC_STOP 3

typedef struct{
	uint32_t cr3,esp,ebp,eip;
	uint32_t eax,ebx,ecx,edx;

	uint32_t k_esp;
}context_t;

typedef struct{
	uint32_t pid;
	char name[250];
	fs_node_t* node;
	context_t* state;
	uint32_t status;
}proc_t;


void schedule(regs_t reg);

proc_t* create_process_from_routine(const char* name,void* routine,uint8_t sched);
proc_t* create_process(fs_node_t* file);
void setup_ctx(context_t* ctx,regs_t r);

void exit(uint32_t pid);

extern void jump_usermode(uint32_t entry);

uint32_t get_current_pid();
void clean_process(proc_t* proc);
