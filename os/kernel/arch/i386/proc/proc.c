/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

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

int32_t free_pid(){
	for(int32_t i = 0;i<MAX_PROCESSES;i++){
		if(!processes[i]){
			return i;
		}
	}
	return -1;
}

proc_t* create_process_from_routine(const char* name,void* routine,uint8_t sched){
	asm("cli");
	proc_t* new_proc = kmalloc(sizeof(proc_t));
	new_proc->state = kmalloc(sizeof(context_t));
	
	if(routine){
		new_proc->state->eip = (uint32_t)routine;
		void* stack = kmalloc(4096);
		memset(stack,0,4096);
		new_proc->state->esp = (uint32_t)stack+4096;
		new_proc->state->ebp = new_proc->state->esp;
		new_proc->state->esp -= sizeof(uintptr_t);
		*((uintptr_t *)new_proc->state->esp) = 0xDEADBEEF;
	}
	
	
	new_proc->state->cr3 = copy_page_directory(kernel_page_directory);
	new_proc->status = routine?PROC_RUN:PROC_CREATED;
	
	new_proc->pid = free_pid();
	memcpy(new_proc->name,name,strlen(name));
	
	if(sched){
		processes[new_proc->pid] = new_proc;
		
		total_prcs++;
		
		kinfo("Process created: '%s' with pid %d (stack %a)\n",name,new_proc->pid,new_proc->state->ebp);
		asm("sti");
	}	
	
	return new_proc;
}

proc_t* create_process(fs_node_t* node,uint8_t sched){
	//kinfo("HERE\n");
	asm("cli");
	uint8_t* buffer = kmalloc(node->size); //TODO load only header
	if(!knread(node,0,node->size,buffer)){
		kerr("Failed to read exec file\n");
		return 0;
	}
	proc_t* proc = create_process_from_routine(node->name,0,0);
	
	set_page_directory(proc->state->cr3);
	knpalloc(USER_STACK);
	proc->state->esp = USER_STACK + 4096;
	proc->state->ebp = proc->state->esp;
	uint32_t entry = elf_load_file(buffer);
	proc->state->eip = entry;
	kinfo("ENTRY: %a\n",entry);
	kfree(buffer);
	if(!entry || entry == 1){
		kerr("Failed to load exec file!");
		return 0;
	}
	if(sched){
		processes[proc->pid] = proc;
		
		total_prcs++;
		
		kinfo("Process created: '%s' with pid %d (stack %a)\n",node->name,proc->pid,proc->state->ebp);
		asm("sti");
	}
	return proc;
}


void idle(){
	//kinfo("IDLE\n");
	while(1){
		asm("hlt");
	}
}

void init_sched(){
	asm("cli");
	create_process_from_routine("kidle",&idle,1); //spawn kernel idle process
	//create_process(kseek("/root/test.smp"));
	asm("sti");
}

void schedule(regs_t reg){
	
	if(total_prcs){
		if(current_piid >= 0){
			proc_t* current = processes[current_piid];
			save_ctx(current->state,reg);
			
		}
		do{
			current_piid++;
			if(current_piid >= MAX_PROCESSES){
				current_piid = 0;
			}
		}while(!processes[current_piid]);
		setup_ctx(processes[current_piid]->state,reg);
	}
}

void kill(uint32_t pid){
	asm("cli");
	kinfo("Killing %d\n",pid);
	if(pid < MAX_PROCESSES && processes[pid]){
		processes[pid] = 0;
		total_prcs--;
		current_piid = -1;
	}
	asm("sti");
}
