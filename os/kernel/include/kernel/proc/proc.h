/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/fs/vfs.h>

#include <kernel/global.h>

#define MAX_PROCESSES 256

#define PROC_INVALID 0
#define PROC_READY   1
#define PROC_STOP    2
#define PROC_WAIT    3

#define PROC_PRIORITY_HIGH 1
#define PROC_PRIORITY_LOW  0

typedef struct{
	uint32_t cr3,esp,ebp,eip;
	uint32_t eax,ebx,ecx,edx;

	uint32_t k_esp;
}context_t;

typedef struct{
	uint32_t pid;
	char name[64];
	fs_node_t* node;
	context_t* state;
	regs_t syscall_state;
	regs_t signal_state;
	uint8_t priority;
	uint8_t* heap;
	uint32_t old_heap;
	uint32_t heap_size;
	uint32_t status;
	uint32_t queue_idx;
	fs_node_t** fswait_nodes;
	uint32_t fswait_nodes_cnt;
	fs_node_t** f_descs; //opened file descriptors (this returned by open() syscall)
	uint32_t  f_descs_cnt;
}proc_t;


void schedule(regs_t reg,uint8_t save);

proc_t* create_child(proc_t* parent);
uint32_t fork();

void setup_ctx(context_t* ctx,regs_t r);

void exit(proc_t* pid);

extern void jump_usermode(uint32_t entry);

proc_t* get_current_process();
proc_t* get_process_by_pid(uint32_t pid);
void clean_process(proc_t* proc);

void process_fswait(proc_t* proc, fs_node_t** nodes, uint32_t cnt);
void process_fswait_notify(proc_t* proc,fs_node_t* node);

proc_t* execute(fs_node_t* node,char** argv,char** envp,uint8_t init); //executes file
