/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/proc/proc.h>
#include <kernel/memory/memory.h>

static proc_t* processes[MAX_PROCESSES];

static proc_t* ready_procs_high[MAX_PROCESSES];
static proc_t* ready_procs_low[MAX_PROCESSES];
static proc_t* wait_procs[MAX_PROCESSES];
static proc_t* killed_procs[MAX_PROCESSES];

static proc_t* current_process =0;
static proc_t* init_process = 0;
static uint32_t total_prcs = 0;

static uint32_t ready_procs_pointer_high = 0;
static uint32_t ready_procs_pointer_low  = 0;

static uint32_t ready_procs_size_high = 0;
static uint32_t ready_procs_size_low  = 0;

static uint32_t wait_procs_size = 0;
static uint32_t killed_procs_size = 0;

proc_t* get_ready_high(){
	if(ready_procs_size_high == 0){
		return 0;
	}
	uint32_t first = ready_procs_pointer_high;
	proc_t* proc = 0;
	do{
		proc = ready_procs_high[ready_procs_pointer_high];
		ready_procs_pointer_high++;
		if(first == ready_procs_high){
			break;
		}
		if(ready_procs_pointer_high > ready_procs_size_high){
			ready_procs_pointer_high = 0;
		}
	}while(!proc);
	return proc;
}


proc_t* get_ready_low(){
	if(ready_procs_size_low == 0){
		return 0;
	}
	uint32_t first = ready_procs_pointer_low;
	proc_t* proc = 0;
	do{
		proc = ready_procs_low[ready_procs_pointer_low];
		ready_procs_pointer_low++;
		if(first == ready_procs_low){
			break;
		}
		if(ready_procs_pointer_low > ready_procs_size_low){
			ready_procs_pointer_low = 0;
		}
	}while(!proc);
	return proc;
}

void ready_insert(proc_t* proc){
	proc->status = PROC_READY;
	if(proc->priority == PROC_PRIORITY_HIGH){
		for(uint32_t i =0;i<ready_procs_size_high;i++){
			if(!ready_procs_high[i]){
				ready_procs_high[i]= proc;
				proc->queue_idx = i;
				return;
			}
		}
		if(ready_procs_size_high < MAX_PROCESSES){
			proc->queue_idx = ready_procs_size_high;
			ready_procs_high[ready_procs_size_high] = proc;
			ready_procs_size_high++;
		}
	}else{
		for(uint32_t i =0;i<ready_procs_size_low;i++){
			if(!ready_procs_low[i]){
				ready_procs_low[i]= proc;
				proc->queue_idx = i;
				return;
			}
		}
		if(ready_procs_size_low < MAX_PROCESSES){
			//kinfo("Inserted %s: %d\n",proc->name,ready_procs_size_low);
			proc->queue_idx = ready_procs_size_low;
			ready_procs_low[ready_procs_size_low] = proc;
			ready_procs_size_low++;
		}
	}
}

void ready_remove(proc_t* proc){
	if(proc->priority == PROC_PRIORITY_HIGH){
		ready_procs_high[proc->queue_idx] = 0;
	}else{
		//kinfo("Removed %d from ready\n",proc->queue_idx);
		ready_procs_low[proc->queue_idx] = 0;
	}
}

void wait_insert(proc_t* proc){
	proc->status = PROC_WAIT;
	for(uint32_t i =0;i<wait_procs_size;i++){
		if(!wait_procs[i]){
			wait_procs[i]= proc;
			proc->queue_idx = i;
			return;
		}
	}
	if(wait_procs_size < MAX_PROCESSES){
		proc->queue_idx = wait_procs_size;
		wait_procs[wait_procs_size] = proc;
		wait_procs_size++;
	}
}

void wait_remove(proc_t* proc){
	wait_procs[proc->queue_idx] = 0;
}

void killed_insert(proc_t* proc){
	killed_procs[killed_procs_size] = proc;
	killed_procs_size++;
}

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
	 
	//kinfo("EIP -> %a\n",r->eip);
	
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
	
	//kinfo("EIP <- %a\n",r->eip);
}

int32_t free_pid(){
	for(int32_t i = 0;i<MAX_PROCESSES;i++){
		if(!processes[i]){
			return i;
		}
	}
	return -1;
}


proc_t* create_process(const char* name, proc_t* parent, uint8_t clone){
	proc_t* new_proc = kmalloc(sizeof(proc_t));
	memset(new_proc,0,sizeof(proc_t));
	new_proc->state = kmalloc(sizeof(context_t));
	memset(new_proc->state,0,sizeof(context_t));
	new_proc->syscall_state = 0;
	
	new_proc->state->cr3 = clone?copy_page_directory(parent->state->cr3):copy_page_directory(kernel_page_directory);
	new_proc->state->k_esp = (uint32_t)kvalloc(4096,4096) + 4096;
	
	new_proc->pid = free_pid();
	
	new_proc->priority = parent?parent->priority:PROC_PRIORITY_LOW;
	if(!clone){
		strcpy(new_proc->name,name);
		new_proc->heap = USER_HEAP;
		set_page_directory(new_proc->state->cr3);
		for(uint32_t i=0;i<USER_HEAP_SIZE;i+=4096){
			knpalloc(USER_HEAP + i);
		}
		new_proc->old_heap = new_proc->heap;
		new_proc->heap_size = USER_HEAP_SIZE;
		knpalloc(USER_STACK);
		if(current_process){
			set_page_directory(current_process->state->cr3);
		}
	}else{
		
		strcpy(new_proc->name,parent->name);
		new_proc->heap = parent->heap;
		new_proc->old_heap = parent->old_heap;
		new_proc->heap_size = parent->heap_size;
	}
	
	if(!clone){
		new_proc->state->esp = USER_STACK + 4096;
		new_proc->state->ebp = new_proc->state->esp;
		new_proc->state->eip = 0;
	}else{
		new_proc->state->esp = parent->syscall_state->esp;
		new_proc->state->ebp = parent->syscall_state->ebp;
		new_proc->state->eip = parent->syscall_state->eip;
		
		kinfo("Cloned process will jump to %a(par = %a)\n",new_proc->state->eip,parent->syscall_state->eip);
	}
	
	

	processes[new_proc->pid] = new_proc;
	total_prcs++;
	ready_insert(new_proc);
		
	kinfo("Process created: '%s' with pid %d (stack %a)\n",new_proc->name,new_proc->pid,new_proc->state->k_esp);
	return new_proc;
}



proc_t* execute(fs_node_t* node,uint8_t init){
	asm("cli");
	kinfo("Creating process from node %s\n",node->name);
	uint8_t* buffer = kmalloc(node->size); //TODO load only header
	if(!kread(node,0,node->size,buffer)){
		kerr("Failed to read exec file\n");
		return 0;
	}

	proc_t* proc = create_process(node->name,current_process,0);
	
	set_page_directory(proc->state->cr3);
	tss_set_kernel_stack(proc->state->k_esp);
	
	uint32_t entry = elf_load_file(buffer);
	
	if(!init){ //This is hack, TODO rewrite 
		set_page_directory(current_process->state->cr3);
		tss_set_kernel_stack(current_process->state->k_esp);
	}
	
	kinfo("ELF loaded\n");
	kfree(buffer);
	proc->state->eip = entry;
	kinfo("ENTRY: %a\n",entry);
	

	if(!entry || entry == 1){
		kerr("Failed to load exec file!");
		exit(proc);
		return 0;
	}

	if(init){ //This is hack, TODO rewrite 
		init_process = proc;
		current_process = proc;
		jump_usermode(entry);
	}
	
	return proc;
}

void clean_process(proc_t* proc){
	kinfo("Cleaning process: %s(%d)\n",proc->name,proc->pid);
	total_prcs--;
	uint32_t pid = proc->pid;
	kpfree(processes[proc->pid]->state->cr3);
	kvfree(processes[proc->pid]->state->k_esp);
	kfree(processes[proc->pid]->state);
	for(uint32_t i = 0;i<processes[proc->pid]->f_descs_cnt;i++){
		kclose(processes[proc->pid]->f_descs[i]);
	}
	if(processes[proc->pid]->f_descs_cnt){
		kfree(processes[proc->pid]->f_descs);
	}
	kfree(processes[proc->pid]);
	processes[pid] = 0;
}

void clean_processes(){
	for(uint32_t i=0;i<killed_procs_size;i++){
		clean_process(killed_procs[i]);
	}
	memset(killed_procs,0,killed_procs_size);
	killed_procs_size = 0;
}

void schedule(regs_t reg,uint8_t save){
	if(total_prcs){
		if(!reg && current_process && current_process->syscall_state){
			reg = current_process->syscall_state;
		}
		clean_processes();
		if(current_process && save){
			save_ctx(current_process->state,reg);
		}
		proc_t* high = get_ready_high();
		proc_t* low  = get_ready_low();
		
		if(high){
			current_process = high;
			setup_ctx(high->state,reg);
		}else if(low){
			current_process = low;
			setup_ctx(low->state,reg);
		}
		
		//kinfo("Switching to: %d\n",current_process->pid);
	}else{
		memset(processes,0,sizeof(proc_t*)*MAX_PROCESSES);
	}
}

void exit(proc_t* proc){
	if(proc->pid == 0){
		return;
	}
	kinfo("Stopping process %s(%d)\n",proc->name,proc->pid);
	if(proc->status == PROC_READY){
		ready_remove(proc);
	}else if(proc->status == PROC_WAIT){
		wait_remove(proc);
	}
	proc->status = PROC_STOP;
	killed_insert(proc);
	if(proc == current_process){
		current_process = 0;
		schedule(proc->syscall_state,0);
	}
}

proc_t* get_current_process(){
	return current_process;
}

proc_t* get_process_by_pid(uint32_t pid){
	return processes[pid];
}

void process_fswait(proc_t* proc,fs_node_t** nodes, uint32_t cnt){
	if(proc->status != PROC_WAIT && !proc->fswait_nodes_cnt){
		//kinfo("%d\n",proc->status);
		proc->fswait_nodes =  kmalloc(sizeof(fs_node_t*)*cnt);
		memcpy(proc->fswait_nodes,nodes,sizeof(fs_node_t*)*cnt);
		proc->fswait_nodes_cnt = cnt;
		for(uint32_t i=0;i<proc->fswait_nodes_cnt;i++){
			kaddwaiter(proc->fswait_nodes[i],proc);
		}
		if(proc->status == PROC_READY){
			
			ready_remove(proc);
		}
		wait_insert(proc);
		//kinfo("Process %s(%d) now waits for events in %d nodes\n",proc->name,proc->pid,cnt);
		if(proc == current_process){
			//kinfo("SCHED\n");
			schedule(0,1);
		}
	}else{
		//kwarn("Tried to fswait process which already in wait: %s - %d status\n",proc->name,proc->status);
		//while(1);
	}
}

void process_fswait_awake(proc_t* proc){
	proc->fswait_nodes_cnt = 0;
	kfree(proc->fswait_nodes);
	wait_remove(proc);
	ready_insert(proc);
}

void process_fswait_notify(proc_t* process,fs_node_t* node){
	if(process->fswait_nodes_cnt){
		for(uint32_t i=0;i<process->fswait_nodes_cnt;i++){
			if(node->inode == process->fswait_nodes[i]->inode){
				process_fswait_awake(process);
			}
		}
	}
}
