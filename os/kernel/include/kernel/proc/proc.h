/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/fs/vfs.h>

#include <kernel/global.h>

#define MAX_PROCESSES 256

#define PROC_INVALID 0
#define PROC_CREATED 1
#define PROC_READY   2
#define PROC_STOP    3
#define PROC_WAIT    4

#define PROC_PRIORITY_HIGH 1
#define PROC_PRIORITY_LOW  0

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
	uint8_t priority;
	uint8_t* heap;
	uint32_t status;
	uint32_t queue_idx;
	fs_node_t** fswait_nodes;
}proc_t;


void schedule(regs_t reg);

proc_t* create_process_from_routine(const char* name,void* routine,uint8_t sched);
proc_t* create_process(fs_node_t* file);
void setup_ctx(context_t* ctx,regs_t r);

void exit(proc_t* pid);

extern void jump_usermode(uint32_t entry);

proc_t* get_current_process();
void clean_process(proc_t* proc);
