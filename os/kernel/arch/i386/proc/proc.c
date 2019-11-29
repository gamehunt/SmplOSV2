#include <kernel/proc/proc.h>
#include <kernel/memory/memory.h>

void setup_ctx(context_t* ctx,regs_t r){
	
	current_page_directory = ctx->cr3;
	
	tss_set_kernel_stack(ctx->esp);
	set_page_directory(ctx->cr3);
	
	r->useresp = ctx->esp;
	r->ebp = ctx->ebp;
	r->eip = ctx->eip;
	
	//kinfo("ENTER EIP: %a\n",ctx->eip);
}

void save_ctx(context_t* ctx,regs_t r){
	
	ctx->cr3 = current_page_directory;
	
	ctx->esp = r->useresp;
	ctx->ebp = r->ebp;
	ctx->eip = r->eip;
	
	//kinfo("EXIT EIP: %a\n",ctx->eip);

}

proc_t* create_process(const char* name,void* routine){
	proc_t* new_proc = kmalloc(sizeof(proc_t));
	new_proc->state = kmalloc(sizeof(context_t));
	new_proc->state->eip = (uint32_t)routine;
	void* stack = kmalloc(4096);
	memset(stack,4096,0);
	new_proc->state->esp = routine?(uint32_t)stack+4096:0;
	new_proc->state->ebp = routine?new_proc->state->esp:0;
	if(routine){
		new_proc->state->esp -= sizeof(uintptr_t);
		*((uintptr_t *)new_proc->state->esp) = (uintptr_t)0xDEADBEEF; //TODO
		printf("%a\n",*((uintptr_t *)new_proc->state->esp));
	}
	
	new_proc->state->cr3 = copy_page_directory(kernel_page_directory);
	new_proc->status = routine?PROC_RUN:PROC_CREATED;
	
	new_proc->pid = total_prcs;
	memcpy(new_proc->name,name,strlen(name));
	
	processes[total_prcs] = new_proc;
	
	total_prcs++;
	
	kinfo("Process created: %s with pid %d (stack %a)\n",name,new_proc->pid,new_proc->state->ebp);
	
	return new_proc;
}


void idle(){
	while(1){
		return;
		asm("hlt");
	}
}


void init_sched(){
	asm("cli");
	create_process("kidle",&idle); //spawn kernel idle process
	asm("sti");
}

void schedule(regs_t reg){
	if(total_prcs){
		if(current_piid >= 0){
			proc_t* current = processes[current_piid];
			save_ctx(current->state,reg);
			
		}
		current_piid++;
		if(current_piid >= total_prcs){
			current_piid = 0;
		}
		setup_ctx(processes[current_piid]->state,reg);
	}
}
