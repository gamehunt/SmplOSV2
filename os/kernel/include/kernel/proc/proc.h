#include <kernel/fs/vfs.h>

#define MAX_PROCESSES 256

#define PROC_INVALID 0
#define PROC_CREATED 1
#define PROC_RUN 2
#define PROC_STOP 3

typedef struct{
	uint32_t cr3,esp,ebp,eip;
}context_t;

typedef struct{
	uint32_t pid;
	char name[250];
	fs_node_t* node;
	context_t* state;
	uint32_t status;
}proc_t;

proc_t* current_proc;

proc_t* processes[MAX_PROCESSES];

uint32_t current_piid;
uint32_t total_prcs = 0;

void init_sched();
void switch_ctx();

proc_t* create_process(const char* name,void* routine);
