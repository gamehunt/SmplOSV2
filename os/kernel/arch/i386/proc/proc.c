/*	
    Copyright (C) 2020
     
    Author: gamehunt 
*/

#include <kernel/proc/proc.h>
#include <kernel/fs/pipe.h>
#include <kernel/memory/memory.h>

static proc_t* processes[MAX_PROCESSES];

static proc_t* ready_procs_high[MAX_PROCESSES]; //Move this to separate type, really
static proc_t* ready_procs_low[MAX_PROCESSES];
static proc_t* killed_procs[MAX_PROCESSES];
static proc_t* sleep_procs[MAX_PROCESSES];

static proc_t* current_process =0;
static proc_t* init_process = 0;
static uint32_t total_prcs = 0;

static uint32_t ready_procs_pointer_high = 0;
static uint32_t ready_procs_pointer_low  = 0;

static uint32_t ready_procs_size_high = 0;
static uint32_t ready_procs_size_low  = 0;

static uint32_t sleep_procs_size = 0;
static uint32_t killed_procs_size = 0;

static sig_t* proc_signals[MAX_SIG];

#define PUSH(esp,type,value)\
	 esp -= sizeof(type);\
	*((type*)esp) = value; \
	 

proc_t* get_ready_high(){
	if(ready_procs_size_high == 0){
		return 0;
	}
	uint32_t first = ready_procs_pointer_high;
	proc_t* proc = 0;
	do{
		proc = ready_procs_high[ready_procs_pointer_high];
		ready_procs_pointer_high++;
		if(ready_procs_pointer_high >= ready_procs_size_high){
			ready_procs_pointer_high = 0;
		}
		if(first == ready_procs_pointer_high){
			break;
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
		if(ready_procs_pointer_low >= ready_procs_size_low){
			ready_procs_pointer_low = 0;
		}
		if(first == ready_procs_pointer_low){
			break;
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
}

void killed_insert(proc_t* proc){
	proc->status = PROC_DEAD;
	for(uint32_t i =0;i<killed_procs_size;i++){
		if(!killed_procs[i]){
			killed_procs[i]= proc;
			proc->queue_idx = i;
			return;
		}
	}
	if(killed_procs_size < MAX_PROCESSES){
		proc->queue_idx = killed_procs_size;
		killed_procs[killed_procs_size] = proc;
		killed_procs_size++;
	}
}

void killed_remove(proc_t* proc){
	killed_procs[proc->queue_idx] = 0;
}

//TODO optimize
void sleep_insert(proc_t* proc){
	proc->status = PROC_SLEEP;
	if(!sleep_procs_size){
		sleep_procs[0] = proc;
		proc->queue_idx = 0;
		sleep_procs_size++;
	}else{
		uint32_t delta = 0;
		uint32_t idx = 0;
		uint8_t flag = 0;
		for(idx;idx<sleep_procs_size;idx++){
			delta += sleep_procs[idx]->sleep_time;
			if(delta >= proc->sleep_time){
				idx++;
				flag = 1;
				break;
			}
		}
		proc->sleep_time -= delta;
		if(!flag){
			idx = sleep_procs_size;
			sleep_procs_size++;
		}
		uint32_t qidx = idx;
		while(sleep_procs[qidx] && qidx < MAX_PROCESSES-1){
			sleep_procs[qidx+1] = sleep_procs[qidx];
			sleep_procs[qidx+1]->queue_idx = qidx+1;
			sleep_procs[qidx+1]->sleep_time-=proc->sleep_time;
			qidx++;
		}
		sleep_procs[idx] = proc;
		proc->queue_idx = idx;
	}
}

void sleep_remove(proc_t* proc){
	sleep_procs[proc->queue_idx] = 0;
	if(sleep_procs_size > 1){
		for(uint32_t i = proc->queue_idx;i<sleep_procs_size;i++){
		//printf("1:%d\n",i);
			sleep_procs[i] = sleep_procs[i+1];
			sleep_procs[i]->queue_idx = i;
		}
	}
	for(uint32_t i=sleep_procs_size-1;i>=0;i--){ //Probably we should do the same for other queues?
		
		if(!sleep_procs[i]){
			sleep_procs_size--;
		}
		
		if(!sleep_procs_size){
			break;
		}
	}
}

void setup_ctx(regs_t ctx,regs_t r,uint32_t pd,uint32_t ks){
	
	//kinfo("Setting up context: %p %p %p %p\n",ctx,r,pd,ks);
	
	set_page_directory(pd);
	tss_set_kernel_stack(ks);
	
	r->eax = ctx->eax;
	r->ebx = ctx->ebx;
	r->ecx = ctx->ecx;
	r->edx = ctx->edx;
	
	r->edi = ctx->edi;
	r->esi = ctx->esi;
	
	//r->eflags = ctx->eflags;
	 
	r->useresp = ctx->useresp;
	r->ebp     = ctx->ebp;
	r->eip     = ctx->eip;
	
	
}

void save_ctx(regs_t ctx,regs_t r){
	
	/*kinfo("Saving up context: %p into %p\n",r,ctx);
	
	kinfo("[0]Context dump: \n");
	kinfo("\t EIP-> %p \n",r->eip);
	kinfo("\t EBP-> %p \n",r->ebp);
	kinfo("\t ESP-> %p \n",r->useresp);
	kinfo("---------------\n");
	kinfo("\tC EIP-> %p \n",ctx->eip);
	kinfo("\tC EBP-> %p \n",ctx->ebp);
	kinfo("\tC ESP-> %p \n",ctx->useresp);*/
	
	ctx->eax = r->eax;
	ctx->ebx = r->ebx;
	ctx->ecx = r->ecx;
	ctx->edx = r->edx;
	
	ctx->edi = r->edi;
	ctx->esi = r->esi;
	
	//ctx->eflags = r->eflags;
	 
	ctx->useresp = r->useresp;
	ctx->ebp     = r->ebp;
	ctx->eip     = r->eip;
	
	/*r->eip = 0xFFFFFFFF;
	
	kinfo("[1]Context dump: \n");
	kinfo("\t EIP-> %p \n",r->eip);
	kinfo("\t EBP-> %p \n",r->ebp);
	kinfo("\t ESP-> %p \n",r->useresp);
	kinfo("---------------\n");
	kinfo("\tC EIP-> %p \n",ctx->eip);
	kinfo("\tC EBP-> %p \n",ctx->ebp);
	kinfo("\tC ESP-> %p \n",ctx->useresp);*/
	
	//kinfo("EIP <- %a\n",r->eip);
}


//Pid 0 fucks up ACPI
uint32_t free_pid(){
	for(uint32_t i = 1;i<MAX_PROCESSES;i++){
		if(!processes[i]){
			return i;
		}
	}
	return 0;
}


proc_t* create_process(const char* name, proc_t* parent){
	
	kinfo("Allocating process structures\n");
	
	proc_t* new_proc = kmalloc(sizeof(proc_t));
	memset(new_proc,0,sizeof(proc_t));
	
	new_proc->state = kmalloc(sizeof(struct registers));
	memset(new_proc->state,0,sizeof(struct registers));
	
	new_proc->syscall_state = 0;
	new_proc->signal_state = kmalloc(sizeof(struct registers));
	uint32_t cpdir = current_page_directory;
	set_page_directory(kernel_page_directory);
	new_proc->pdir = copy_page_directory(kernel_page_directory);
	new_proc->kernel_stack = (uint32_t)kvalloc(4*4096,4096) + 4*4096;
	memset((uint32_t)new_proc->kernel_stack-4*4096,0,4*4096);
	
	set_page_directory(cpdir);
	
	
	uint32_t pid = free_pid();
	if(!pid){
			crash_info_t crash;
			crash.description = "Failed to allocate pid";
			kpanic(crash);
	}
	
	new_proc->pid = pid;
	
	
	char path[64];
	memset(path,0,64);
	sprintf(path,"/proc/%d",new_proc->pid);
	new_proc->node = kcreate(path,VFS_TYPE_VIRTUAL);
	memset(path,0,64);
	sprintf(path,"/proc/%d/stdout",new_proc->pid);
	pipe_create(path,256);
	memset(path,0,64);
	sprintf(path,"/proc/%d/stderr",new_proc->pid);
	pipe_create(path,128); 
	memset(path,0,64);
	sprintf(path,"/proc/%d/stdin",new_proc->pid);
	pipe_create(path,256); 
	
	new_proc->priority = parent?parent->priority:PROC_PRIORITY_LOW;
	
	new_proc->sig_stack_esp = -1;
	
	new_proc->uid = parent?parent->uid:PROC_ROOT_UID;
	
	if(parent){
		new_proc->work_dir = parent->work_dir;
		memset(new_proc->work_dir_abs,0,256);
		strcpy(new_proc->work_dir_abs,parent->work_dir_abs);
	}else{
		new_proc->work_dir = kseek("/");
		memset(new_proc->work_dir_abs,0,256);
		strcpy(new_proc->work_dir_abs,"/");
	}
	
	strcpy(new_proc->name,name);
	new_proc->heap = USER_HEAP;
	
	
	
	set_page_directory(new_proc->pdir);
	for(uint32_t i=0;i<USER_HEAP_SIZE;i+=4096){
		knpalloc(USER_HEAP + i);
	}
	new_proc->heap_size = USER_HEAP_SIZE;
	knpalloc(USER_STACK);
	if(current_process){
		set_page_directory(current_process->pdir);
	}
	
	new_proc->state->useresp = USER_STACK + 4096;
	new_proc->state->ebp = new_proc->state->useresp;
	new_proc->state->eip = 0;
	

	
	
	if(parent){
		if(parent->child_count){
			parent->childs = krealloc(parent->childs,(parent->child_count+1)*sizeof(proc_t));
		}else{
			parent->childs = kmalloc(sizeof(proc_t));
		}
		parent->child_count++;
		parent->childs[parent->child_count-1] = new_proc;
		new_proc->parent = parent;
	}
	processes[new_proc->pid] = new_proc;
	total_prcs++;
	ready_insert(new_proc);
	
	kinfo("Process created: '%s' with pid %d (stack %p)\n",new_proc->name,new_proc->pid,new_proc->kernel_stack);

	return new_proc;
}



proc_t* execute(fs_node_t* node,char** argv,char** envp,uint8_t init){
	disable_interrupts();
	if(!validate(node)){
		kinfo("Tried to create process from invalid node %p!\n",node);
		return 0;
	}
	kinfo("Creating process from node %s; args: %p %p\n",node->name,argv,envp);
	uint8_t* buffer = kmalloc(node->size); //TODO load only header
	if(!kread(node,0,node->size,buffer)){
		kerr("Failed to read exec file\n");
		return 0;
	}

	proc_t* proc = create_process(node->name,current_process);
	
	int argc = 0;
	if(validate(argv)){
		while(validate(argv[argc])){
			argc++;
		}
	}
	int envsize = 0;
	if(validate(envp)){
		while(validate(envp[envsize])){
			envsize++;
		}
	}
	
	char** argv_copy = 0;
	if(argc){
		argv_copy = kmalloc(sizeof(char*)*argc);
		for(uint32_t i=0;i<argc;i++){
			argv_copy[i] = kmalloc(strlen(argv[i])+1);
			memset(argv_copy[i],0,strlen(argv[i])+1);
			strcpy(argv_copy[i],argv[i]);
		}
	}
	char** envp_copy = 0;
	if(envsize){
		envp_copy = kmalloc(sizeof(char*)*envsize);
		for(uint32_t i=0;i<envsize;i++){
			envp_copy[i] = kmalloc(strlen(envp[i])+1);
			memset(envp_copy[i],0,strlen(envp[i])+1);
			strcpy(envp_copy[i],envp[i]);
		}
	}
	set_page_directory(proc->pdir);
	tss_set_kernel_stack(proc->kernel_stack);
	
	kinfo("Allocating space for %d cmd arguments and %d envp entries\n",argc,envsize);
	
	char** usr_argv = 0;
	char** usr_envp = 0;
	mem_t* allocation = 0;
	//Here we manually manage memory blocks in user heap
	if(argc){
		allocation = USER_HEAP;
		allocation->size =  sizeof(char*)*argc;
		allocation->prev = 0xAABBCCDD;
		allocation->next = 0xAABBCCDD;
		usr_argv = (char**)((uint32_t)allocation + sizeof(mem_t));
		for(uint32_t i=0;i<argc;i++){
			
			allocation = (mem_t*)((uint32_t)allocation + sizeof(mem_t) + allocation->size);
			allocation->size = strlen(argv_copy[i])+1;
			allocation->prev = 0xAABBCCDD;
			allocation->next = 0xAABBCCDD;
			usr_argv[i] = (char*)((uint32_t)allocation + sizeof(mem_t)); 
			memset(usr_argv[i],0,strlen(argv_copy[i])+1);
			strcpy(usr_argv[i],argv_copy[i]);
		}
	}
	if(envp){
		allocation = argc?(mem_t*)((uint32_t)allocation + sizeof(mem_t)+ allocation->size):USER_HEAP;
		allocation->size =  sizeof(char*)*envsize;
		allocation->prev = 0xAABBCCDD;
		allocation->next = 0xAABBCCDD;
		usr_envp = (char**)((uint32_t)allocation + sizeof(mem_t));
		for(uint32_t i=0;i<envsize;i++){
			allocation = (mem_t*)((uint32_t)allocation + sizeof(mem_t) + allocation->size);
			allocation->size = strlen(envp_copy[i])+1;
			allocation->prev = 0xAABBCCDD;
			allocation->next = 0xAABBCCDD;
			usr_envp[i] = (char*)((uint32_t)allocation + sizeof(mem_t));
			memset(usr_envp[i],0,strlen(envp_copy[i])+1);
			strcpy(usr_envp[i],envp_copy[i]);
		}
	}
	if(argv_copy){
		kfree(argv_copy);
	}
	if(envp_copy){
		kfree(envp_copy);
	}
	
	uint32_t entry = elf_load_file(buffer);
	kfree(buffer);
	proc->state->eip = entry;
	kinfo("ENTRY: %p\n",entry);

	PUSH(proc->state->useresp,char**,usr_envp);
	PUSH(proc->state->useresp,char**,usr_argv);
	PUSH(proc->state->useresp,int,   argc);
	
	if(!init){ //This is hack, TODO rewrite 
		set_page_directory(current_process->pdir);
		tss_set_kernel_stack(current_process->kernel_stack);
	}
	
	if(!entry || entry == 1){
		kerr("Failed to load exec file!");
		proc_exit(proc);
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
	
	if(proc->pwait){
		if(validate(proc->parent) && proc->parent->status == PROC_WAIT){
			ready_insert(proc->parent);
			proc->pwait = 0;
		}
	}
	
	uint32_t pid = proc->pid;

	char path[64];
	memset(path,0,64);
	sprintf(path,"/proc/%d",proc->pid);
	fs_node_t* proc_node = kseek(path);
	if(proc_node){
		fs_dir_t* dir = kreaddir(proc_node);
		for(uint32_t i=0;i<dir->chld_cnt;i++){
			char* buff = kmalloc(dir->chlds[i]->size);
			memset(buff,0,dir->chlds[i]->size);
			uint32_t sz = 0;
			if(sz = kread(dir->chlds[i],0,dir->chlds[i]->size,buff)){
				//Stream in proc directory not empty, we shouldn't clean this process yet
				if(dir->chlds[i]->fsid == 1){
					kwrite(dir->chlds[i],0,sz,buff); //Restore pipe values
				}
				kfree(dir);
				kfree(buff);
				return;
			}
			kfree(buff);
			kremove(dir->chlds[i]);
		}
		kremove(proc_node);
	}
	kinfo("Cleaning process: %s(%d) - %p - pwait=%d\n",proc->name,proc->pid,proc,proc->pwait);

	//kffree(processes[proc->pid]->state->cr3);
	kvfree(processes[proc->pid]->kernel_stack);
	kfree(processes[proc->pid]->state);
	kfree(processes[proc->pid]->signal_state);
	if(processes[proc->pid]->sig_stack_esp >= 0){
		kfree(processes[proc->pid]->sig_stack);
	}
	for(uint32_t i = 0;i<processes[proc->pid]->f_descs_cnt;i++){
		if(validate(processes[proc->pid]->f_descs[i])){
			kclose(processes[proc->pid]->f_descs[i]);
		}
	}
	if(processes[proc->pid]->f_descs_cnt && validate(processes[proc->pid]->f_descs)){
		kfree(processes[proc->pid]->f_descs);
	}
		
	
	killed_remove(proc);
	//kfree(processes[proc->pid]); This crashes
	processes[pid] = 0;
	kinfo("Cleanup completed\n");
}

void clean_processes(){
	for(uint32_t i=0;i<killed_procs_size;i++){
		if(killed_procs[i]){
			clean_process(killed_procs[i]);
		}
	}
}

void schedule(regs_t reg,uint8_t save){
	if(total_prcs){
		while(sleep_procs_size){
			sleep_procs[0]->sleep_time--;
			if(!sleep_procs[0]->sleep_time){
				process_awake(sleep_procs[0]);
			}else{
				break;
			}
		}
		if(!reg && current_process && validate(current_process->syscall_state)){
			reg = current_process->syscall_state;
		}else if(!reg){
			//kinfo("No valid registers - skipping tick\n");
			return;
		}
		
		if(current_process && save){
			if(current_process->in_sig){
				save_ctx(current_process->signal_state,reg);
			}else{
				save_ctx(current_process->state,reg);
			}
		}
		proc_t* high = get_ready_high();
		proc_t* low  = get_ready_low();
		
		if(high){
			//kinfo("Scheduling high\n");
			current_process = high;	
		}else if(low){
			//kinfo("Scheduling low\n");
			
			current_process = low;
		}
		
		if(current_process){
			if(current_process->in_sig){
				setup_ctx(current_process->signal_state,reg,current_process->pdir,current_process->kernel_stack);
			}else{
				setup_ctx(current_process->state,reg,current_process->pdir,current_process->kernel_stack);
			}
		}
		
		clean_processes();
		
		//return;

		if(!current_process->in_sig && current_process->sig_stack_esp >= 0){
			
			//kinfo("Handling signal: [%p+%d]\n",current_process->sig_stack,current_process->sig_stack_esp);
			
			uint32_t signum = current_process->sig_stack[current_process->sig_stack_esp];
			current_process->sig_stack_esp--;
			if(current_process->sig_stack_esp < 0){
				kfree(current_process->sig_stack);
			}
			//kinfo("SIGNUM: %d\n",signum);
			if(signum > MAX_SIG){
				kwarn("Tried to handle bad signal: %d\n",signum);
				return;
			}
			
			
			sig_handler_t sig = current_process->sig_handlers[signum];
			
			if(sig){
				current_process->in_sig = 1;
				reg->eip = sig;
			}else if(proc_signals[signum]->unhandled_behav == SIG_UNHANDLD_KILL){
				current_process->syscall_state = reg;
				proc_exit(current_process);
			}
		}
		
	}else{
		memset(processes,0,sizeof(proc_t*)*MAX_PROCESSES);
	}
}

void proc_exit(proc_t* proc){
	if(proc->pid == 1){
		if(proc == current_process){
			current_process = 0;
		}
		schedule(proc->syscall_state,0);
		return;
	}
	kinfo("Stopping process %s(%d)\n",proc->name,proc->pid);
	if(proc->status == PROC_READY){
		ready_remove(proc);
	}
	proc->status = PROC_DEAD;
	if(proc->parent){
		send_signal(proc->parent,SIG_CHILD);
	}
	for(uint32_t i=0;i<proc->child_count;i++){
		proc->childs[i]->parent = 0;
		proc->childs[i]->pwait = 0;
	}
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
		proc->fswait_nodes = nodes;
		proc->fswait_nodes_cnt = cnt;
		for(uint32_t i=0;i<proc->fswait_nodes_cnt;i++){
			pipe_add_waiter(proc->fswait_nodes[i],proc); //TODO select_fs
		}
		if(proc->status == PROC_READY){
			ready_remove(proc);
		}
		wait_insert(proc);
		if(proc == current_process){
			schedule(0,1);
		}
	}
}

void process_fswait_awake(proc_t* proc){
	for(uint32_t i=0;i<proc->fswait_nodes_cnt;i++){
		pipe_remove_waiter(proc->fswait_nodes[i],proc);
	}
	proc->fswait_nodes_cnt = 0;
	kfree(proc->fswait_nodes);
	ready_insert(proc);
}

void process_fswait_notify(proc_t* process,fs_node_t* node){
	//kinfo("Notifying %s\n",process->name);
	if(validate(process) && validate(node) && process->fswait_nodes_cnt && validate(process->fswait_nodes)){
		for(uint32_t i=0;i<process->fswait_nodes_cnt;i++){
			if(validate(process->fswait_nodes[i]) && node->inode == process->fswait_nodes[i]->inode){
				process->state->eax = i; //Return which node awoken us
				//kinfo("Set eax of %s to %d\n",process->name,process->state->eax);
				process_fswait_awake(process);
			}else if(!validate(process->fswait_nodes[i])){
				kwarn("Bad fswait node: %p\n",process->fswait_nodes[i]);
			}
		}
	}
}

void process_waitpid(proc_t* proc,uint32_t pid){
	
	if(!proc->child_count){
		return;
	}
	
	if(get_process_by_pid(pid) && get_process_by_pid(pid)->parent == proc){
			get_process_by_pid(pid)->pwait = 1;
			ready_remove(proc);
			wait_insert(proc);
			if(proc == current_process){
				schedule(0,1);
			}
	}
}

void send_signal(proc_t* proc,uint32_t sig){
	
	if(!validate(proc)){
		return;
	}
	if(proc == current_process){ //Bad idea for now
		return;
	}
	if(proc->sig_stack_esp >= MAX_SIGSTACK_SIZE){
		return;
	}
	
	proc->sig_stack_esp++;
	if(!proc->sig_stack_esp){
		proc->sig_stack = kmalloc(sizeof(uint32_t));
	}else{
		proc->sig_stack = krealloc(proc->sig_stack,(proc->sig_stack_esp+1)*sizeof(uint32_t));
	}
	if(proc->status == PROC_WAIT){
		proc->sig_ret_state = PROC_WAIT;
		if(proc_signals[sig]->block_behav == SIG_BLOCK_AWAKE){
			proc->sig_stack[proc->sig_stack_esp] = sig;
			ready_insert(proc);
		}
		else if(proc_signals[sig]->block_behav == SIG_BLOCK_KILL){
			proc->sig_stack[proc->sig_stack_esp] = sig;
			proc_exit(proc);
		}
		else if(proc_signals[sig]->block_behav == SIG_BLOCK_SKIP){
			proc->sig_stack_esp--;
			if(proc->sig_stack_esp < 0){
				kfree(proc->sig_stack);
			}
		}else{
			proc->sig_stack[proc->sig_stack_esp] = sig;
		}
	}else{
			proc->sig_stack[proc->sig_stack_esp] = sig;
	}

	
	//return;
	//kinfo("SIG\n");
	//while(1);
}

void exit_sig(proc_t* p){
	p->in_sig = 0;
	if(p->sig_ret_state == PROC_WAIT){
		ready_remove(p);
		wait_insert(p);
	}
	schedule(0,0);
}

void sig_register(uint32_t sig, sig_t* ss){
	if(sig < 64){
		proc_signals[sig] = ss;
	}
}

void init_signals(){
	sig_t* sigchld = kmalloc(sizeof(sig_t));
	strcpy(sigchld->name,"SIGCHILD");
	sigchld->unhandled_behav = SIG_UNHANDLD_IGNORE;
	sigchld->block_behav     = SIG_BLOCK_AWAKE;
	sigchld->sig_num = SIG_CHILD;
	sig_register(SIG_CHILD,sigchld);
}

void set_sig_handler(proc_t* proc,sig_handler_t handl,uint32_t sig){
	if(sig>=MAX_SIG){
		return;
	}
	kinfo("Signal hadnler set: %d for %s\n",sig,proc->name);
	proc->sig_handlers[sig] = handl;
}

void process_sleep(proc_t* proc, uint32_t ticks){
	if(proc->status == PROC_READY){
		ready_remove(proc);
	}
	proc->sleep_time = ticks;
	sleep_insert(proc);
	
	if(proc == current_process){
		schedule(0,1);
	}
}

void process_awake(proc_t* proc){
	sleep_remove(proc);
	ready_insert(proc);
}

