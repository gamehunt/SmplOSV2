#include <kernel/proc/sched.h>
#include <kernel/memory/memory.h>
#include <kernel/misc/log.h>

static process_t* processes;
static uint32_t processes_total;

void write_context(context_t* ctx){
	memset(ctx,0,sizeof(context_t));
	asm ("mov %%esp, %0":"=r"(ctx->esp));
	ctx->eip = get_eip();
	asm ("mov %%eax, %0":"=r"(ctx->eip));
	asm ("mov %%ebp, %0":"=r"(ctx->ebp));
	asm ("mov %%cr3, %0":"=r"(ctx->cr3));
}

void ctx_dump(context_t* ctx){
	//terminal_putchar('c');
	kerr("Requested context dump:\n");
	//kerr("EIP: 0x%x\n",ctx->eip);
	//kerr("ESP: 0x%x\n",ctx->esp);
	//kerr("EBP: 0x%x\n",ctx->ebp);
	//kerr("CR3: 0x%x\n",ctx->cr3);
}

void init_sched(){
	context_t* c = kmalloc(sizeof(context_t));
	write_context(c);
	ctx_dump(c);
}
