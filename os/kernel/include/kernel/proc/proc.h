#include <kernel/fs/vfs.h>

#include <kernel/global.h>

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

int32_t current_piid =-1;
uint32_t total_prcs = 0;

void schedule(regs_t reg);

proc_t* create_process(const char* name,void* routine);
void setup_ctx(context_t* ctx,regs_t r);
