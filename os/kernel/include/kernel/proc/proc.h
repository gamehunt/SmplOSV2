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

typedef struct{
	regs_t state;
	regs_t syscall_state;
	uint32_t kernel_stack;
}thread_t;


struct process;

typedef struct{
	uint32_t gid;
	uint8_t priority;
	struct process** processes;
	struct process** queue;
	uint32_t pointer;
	uint32_t ready_count;
	uint32_t queue_size;
	uint32_t group_size;
	uint32_t proc_count;
}thread_group_t;

typedef struct{
	uint32_t size;
	uint32_t offset;
}shmem_block_t;


struct process{
	uint32_t pid;
	thread_group_t* group;
	uint32_t gid;
	char name[64];
	fs_node_t* node;
	thread_t* thread;
	int status;
	uint32_t pdir;
	regs_t signal_state;
	sig_handler_t sig_handlers[MAX_SIG];
	uint32_t* sig_stack; //stack of signals
	long      sig_stack_esp;
	uint8_t in_sig; //Are we handling signal?
	uint8_t* heap;
	uint32_t heap_size;
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
	shmem_block_t** shmem_blocks;
	uint32_t shmem_size;
	uint32_t shmem_bytes;
};

typedef struct process proc_t;


void sig_register(uint32_t sig,sig_t* sigs);
void init_signals();

void schedule(regs_t reg,uint8_t save);

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

void process_create_thread(proc_t* parent,uint32_t entry);

uint32_t process_create_shared(proc_t* proc,uint32_t buffer_size);
shmem_block_t* process_get_shared(proc_t* proc,uint32_t id);
uint32_t process_open_shmem(proc_t* proc,proc_t* target,uint32_t id);
void  process_reset_shmem(proc_t* proc);
