#include <kernel/proc/proc.h>
#include <kernel/global.h>

void setup_ctx(context_t* ctx,regs_t r){
	tss_set_kernel_stack(ctx->esp);
	
	r->useresp = ctx->esp;
	r->ebp = ctx->ebp;
	r->eip = ctx->eip;
}

void setup_next(regs_t r){
		static uint8_t s = 0;
		
		if(s){
			current_proc->state->eip = r->eip;
			current_proc->state->esp = r->useresp;
			current_proc->state->ebp = r->ebp;
		}else{
			s = 1;
		}
		
		current_piid++;
		
		if(current_piid >= total_prcs){
			current_piid = 0;
		}
		current_proc = processes[current_piid];
		setup_ctx(current_proc->state,r);
}

void kidle(){
	while(1){
		asm("hlt");
	}
}

proc_t* create_process(const char* name,void* routine){
	proc_t* new_proc = kmalloc(sizeof(proc_t));
	new_proc->state = kmalloc(sizeof(context_t));
	new_proc->state->eip = (uint32_t)routine;
	new_proc->state->esp = routine?(uint32_t)kmalloc(4096):0;
	new_proc->state->ebp = routine?new_proc->state->esp:0;
	
	new_proc->status = routine?PROC_RUN:PROC_CREATED;
	
	new_proc->pid = total_prcs;
	memcpy(new_proc->name,name,strlen(name));
	
	processes[total_prcs] = new_proc;
	
	total_prcs++;
	
	kinfo("Process created: %s with pid %d\n",name,new_proc->pid);
	
	return new_proc;
}


void init_sched(){
	asm("cli");

	current_proc = create_process("[kidle]",kidle);
	current_piid = total_prcs - 1;
	
	asm("sti");
}

void __schedule(regs_t r){
	setup_next(r);
}
