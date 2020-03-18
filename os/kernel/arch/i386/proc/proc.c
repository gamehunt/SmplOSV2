/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/proc/proc.h>
#include <kernel/memory/memory.h>


static proc_t* current_proc;

static proc_t* processes[MAX_PROCESSES];

static int32_t current_piid =0;
static uint32_t total_prcs = 0;

void setup_ctx(context_t* ctx,regs_t r){
	
	set_page_directory(ctx->cr3);
	tss_set_kernel_stack(ctx->k_esp);
	
	r->useresp = ctx->esp;
	r->ebp = ctx->ebp;
	r->eip = ctx->eip;
	
	r->eax = ctx->eax;
	r->ebx = ctx->ebx;
	r->ecx = ctx->ecx;
	r->edx = ctx->edx;
	
	//kinfo("%a\n",ctx->eip);
}

void save_ctx(context_t* ctx,regs_t r){
	
	ctx->cr3 = current_page_directory;
	
	ctx->esp = r->useresp;
	ctx->ebp = r->ebp;
	ctx->eip = r->eip;
	
	ctx->eax = r->eax;
	ctx->ebx = r->ebx;
	ctx->ecx = r->ecx;
	ctx->edx = r->edx;
	
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
	
	new_proc->state->k_esp = (uint32_t)kvalloc(4096,4096) + 4096;
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

//Currently it works wrong for scheduling new processes TODO: rewrite scheduler
proc_t* create_process(fs_node_t* node){
	asm("cli");
	//kinfo("HERE\n");
	uint8_t* buffer = kmalloc(node->size); //TODO load only header
	if(!kread(node,0,node->size,buffer)){
		kerr("Failed to read exec file\n");
		return 0;
	}
	proc_t* proc = create_process_from_routine(node->name,0,0);

	set_page_directory(proc->state->cr3);
	tss_set_kernel_stack(proc->state->k_esp);
	knpalloc(USER_STACK);

	
	proc->state->esp = USER_STACK + 4096;
	proc->state->ebp = proc->state->esp;
	
	
	uint32_t entry = elf_load_file(buffer);
	kfree(buffer);
	proc->state->eip = entry;
	kinfo("ENTRY: %a\n",entry);
	//kfree(buffer);
	if(proc->pid != 0){ //This is hack, TODO rewrite 
		set_page_directory(processes[current_piid]->state->cr3);
		tss_set_kernel_stack(processes[current_piid]->state->k_esp);
	}
	if(!entry || entry == 1){
		kerr("Failed to load exec file!");
		return 0;
	}
	processes[proc->pid] = proc;
		
	total_prcs++;
	
	if(current_piid < 0){
		current_piid = 0;
	}	
		
	kinfo("Process created: '%s' with pid %d (stack %a)\n",node->name,proc->pid,proc->state->k_esp);
	if(proc->pid == 0){ //This is hack, TODO rewrite 
		jump_usermode(entry);
	}
	return proc;
}

void clean_process(proc_t* proc){
	kinfo("Cleaning process: %s(%d)\n",proc->name,proc->pid);
	total_prcs--;
	uint32_t pid = proc->pid;
	kfree(processes[proc->pid]);
	processes[pid] = 0;
}

//Currently in crashed on second switch TODO: fix
void schedule(regs_t reg){
	asm("cli");
	if(total_prcs){
		int32_t ppid = current_piid;
		if(current_piid >= 0 && processes[current_piid]->status != PROC_STOP){
			proc_t* current = processes[current_piid];
			save_ctx(current->state,reg);
		}else if(current_piid >= 0){
            clean_process(processes[current_piid]);
		}
		do{
			
			current_piid++;
			if(current_piid >= MAX_PROCESSES){
				current_piid = 0;
			}
			if(processes[current_piid] && processes[current_piid]->status == PROC_STOP){
				clean_process(processes[current_piid]);
			}
		}while(!processes[current_piid] || processes[current_piid]->status == PROC_STOP);
		kinfo("Switching from %d to %d\n",ppid,current_piid);
		setup_ctx(processes[current_piid]->state,reg);
	}else{
		memset(processes,0,sizeof(proc_t*)*MAX_PROCESSES);
	}
	asm("sti");
}

void exit(uint32_t pid){
	if(pid == 0){
		return;
	}
	kinfo("Process %s(%d) marked for stop\n",processes[pid]->name,pid);
	if(pid < MAX_PROCESSES && processes[pid]){
		processes[pid]->status = PROC_STOP;
	}
}

uint32_t get_current_pid(){
	return current_piid;
}
