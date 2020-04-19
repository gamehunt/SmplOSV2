/*	
    Copyright (C) 2020
     
    Author: gamehunt 
*/

#pragma once

#include <kernel/fs/vfs.h>

#include <kernel/global.h>

#define MAX_PROCESSES 256

#define PROC_INVALID 0
#define PROC_READY   1
#define PROC_DEAD    2
#define PROC_WAIT    3
#define PROC_SLEEP   4

#define PROC_PRIORITY_HIGH 1
#define PROC_PRIORITY_LOW  0

#define MAX_SIGSTACK_SIZE   64

#define SIG_UNHANDLD_IGNORE 0
#define SIG_UNHANDLD_KILL   1

#define SIG_BLOCK_WAIT  0
#define SIG_BLOCK_SKIP  1
#define SIG_BLOCK_KILL  2
#define SIG_BLOCK_AWAKE 3

#define PROC_ROOT_UID 0

#define SIG_CHILD 0

#define MAX_SIG 64

typedef struct{
	char name[32];
	uint32_t sig_num;
	uint8_t unhandled_behav; //Behaviour if unhandled
	uint8_t block_behav; //Behaviour if blocked
}sig_t;

typedef int (*sig_handler_t)(void);

struct process{
	uint32_t pid;
	char name[64];
	fs_node_t* node;
	regs_t state;
	uint32_t kernel_stack;
	uint32_t pdir;
	regs_t syscall_state;
	uint32_t syscall_ret;
	regs_t signal_state;
	sig_handler_t sig_handlers[MAX_SIG];
	uint32_t* sig_stack; //stack of signals
	long      sig_stack_esp;
	uint8_t in_sig; //Are we handling signal?
	uint8_t priority;
	uint8_t* heap;
	uint32_t heap_size;
	int      status;
	uint8_t sig_ret_state; //Status to which we return after signal;
	uint32_t queue_idx; //Index in queues
	fs_node_t** fswait_nodes; //Nodes which we are wait
	uint32_t fswait_nodes_cnt;
	fs_node_t** f_descs; //opened file descriptors (this returned by open() syscall)
	uint32_t  f_descs_cnt;
	struct process*   parent;
	struct process**  childs;
	uint32_t  child_count;
	uint8_t   pwait; //Is parent awaiting us?
	char     work_dir_abs[256]; //Absolute path to work dir
	fs_node_t* work_dir;
	uint32_t uid; //User id
	uint32_t sleep_time; //TIme until awake (usually delta in sleep queue);
};

typedef struct process proc_t;


void sig_register(uint32_t sig,sig_t* sigs);
void init_signals();

void schedule(regs_t reg,uint8_t save);

proc_t* create_child(proc_t* parent);

void save_ctx(regs_t ctx,regs_t r);
void setup_ctx(regs_t ctx,regs_t r,uint32_t pd,uint32_t ks);

void proc_exit(proc_t* pid);

extern void jump_usermode(uint32_t entry);

proc_t* get_current_process();
proc_t* get_process_by_pid(uint32_t pid);
void clean_process(proc_t* proc);

void process_fswait(proc_t* proc, fs_node_t** nodes, uint32_t cnt);
void process_fswait_notify(proc_t* proc,fs_node_t* node);
void process_waitpid(proc_t* proc,uint32_t pid); //Set proc wait for child pid
void process_sleep(proc_t* proc, uint32_t ticks);
void process_awake(proc_t* proc);

proc_t* execute(fs_node_t* node,char** argv,char** envp,uint8_t init); //executes file

void send_signal(proc_t* proc,uint32_t sig);
void exit_sig(proc_t* proc);
void set_sig_handler(proc_t* proc,sig_handler_t handl,uint32_t sig);
