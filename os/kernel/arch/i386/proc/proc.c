/*	
    Copyright (C) 2020
     
    Author: gamehunt 
    
    
    TODO: now I need to implement some kind of group scheduling
*/

#include <kernel/proc/proc.h>
#include <kernel/fs/pipe.h>
#include <kernel/memory/memory.h>

static proc_t* processes[MAX_PROCESSES];

static proc_t* killed_procs[MAX_PROCESSES];
static proc_t* sleep_procs[MAX_PROCESSES];

static proc_t* current_process =0;
static proc_t* init_process = 0;
static uint32_t total_prcs = 0;

static uint32_t sleep_procs_size = 0;
static uint32_t killed_procs_size = 0;

static sig_t* proc_signals[MAX_SIG];

static thread_group_t* groups[MAX_PROCESSES];
static uint32_t groups_size = 0;
static uint32_t groups_ptr = 0;

#define PUSH(esp,type,value)\
	 esp -= sizeof(type);\
	*((type*)esp) = value; \
	 

void add_group(thread_group_t* group){
	for(uint32_t i=0;i<groups_size;i++){
		if(!groups[i]){
			groups[i] = group;
			group->gid = i;
			return;
		}
	}
	groups[groups_size] = group;
	groups_size++;
}

void remove_group(thread_group_t* group){
	groups[group->gid] = 0;
}

thread_group_t* get_group(uint8_t prior){
	if(groups_size == 0){
		return 0;
	}
	uint32_t first = groups_ptr;
	thread_group_t* gr = 0;
	while(!gr || gr->priority != prior || !gr->ready_count){
		gr = groups[groups_ptr];
		groups_ptr++;
		if(groups_ptr >= groups_size){
			groups_ptr = 0;
		}
		if(first == groups_ptr){
			break;
		}
	}
	if(gr && gr->priority == prior && gr->ready_count){
		return gr;
	}
	return 0;
}

proc_t* get_ready(){
	
	thread_group_t* grp = get_group(PROC_PRIORITY_HIGH);
	if(!grp){
		grp = get_group(PROC_PRIORITY_LOW);
		if(!grp){
			return 0;
		}
	}
	
	proc_t** queue = grp->queue;
	uint32_t ptr = grp->pointer;
	
	uint32_t first = ptr;
	
	proc_t* proc = 0;
	
	while(!proc){
		proc = queue[ptr];
		ptr++;
		if(ptr>=grp->queue_size){
			ptr=0;
		}
		if(ptr==first){
			break;
		}
	}
	
	grp->pointer = ptr;
	
	return proc;
}

void ready_insert(proc_t* proc){
	proc->status = PROC_READY;
	proc_t** queue = proc->group->queue;
	proc->group->ready_count++;
	for(uint32_t i=0;i<proc->group->queue_size;i++){
		if(!queue[i]){
			proc->queue_idx = i;
			queue[i] = proc;
			return;
		}
	}
	
	proc->group->queue_size+=1;
	proc->group->queue = krealloc(proc->group->queue,proc->group->queue_size*sizeof(proc_t*));
	proc->group->queue[proc->group->queue_size-1] = proc;
	proc->queue_idx = proc->group->queue_size-1;
}

void group_insert(proc_t* proc){
	thread_group_t* gr= proc->group;
	gr->proc_count++;
	for(uint32_t i=0;i<gr->group_size;i++){
		if(!gr->processes[i]){
			gr->processes[i] = proc;
			proc->gid = i;
			return;
		}
	}
	gr->processes = krealloc(gr->processes,(gr->group_size+1)*(sizeof(proc_t*)));
	gr->processes[gr->group_size] = proc;
	proc->gid = gr->group_size;
	gr->group_size++;
}

void group_remove(proc_t* proc){
	thread_group_t* gr = proc->group;
	gr->proc_count--;
	if(gr->proc_count <= 0){
		remove_group(gr);
		kfree(gr);
	}else{
		gr->processes[proc->gid] = 0;
	}
}

void ready_remove(proc_t* proc){
	proc->group->ready_count--;
	proc->group->queue[proc->queue_idx] = 0;
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
	
	set_page_directory(pd,0);
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
	
	ctx->eax = r->eax;
	ctx->ebx = r->ebx;
	ctx->ecx = r->ecx;
	ctx->edx = r->edx;
	
	ctx->edi = r->edi;
	ctx->esi = r->esi;

	ctx->useresp = r->useresp;
	ctx->ebp     = r->ebp;
	ctx->eip     = r->eip;
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
	
	new_proc->thread = kmalloc(sizeof(thread_t));
	memset(new_proc->thread,0,sizeof(thread_t)); //TODO calloc for setting memory to zero on allocation
	
	new_proc->thread->state = kmalloc(sizeof(struct registers));
	memset(new_proc->thread->state,0,sizeof(struct registers));
	
	new_proc->thread->syscall_state = 0;
	new_proc->signal_state = kmalloc(sizeof(struct registers));
	uint32_t cpdir = current_page_directory;
	set_page_directory(kernel_page_directory,0);
	new_proc->pdir = copy_page_directory(kernel_page_directory);
	new_proc->thread->kernel_stack = (uint32_t)kvalloc(KERNEL_STACK_PER_PROCESS,4096) + KERNEL_STACK_PER_PROCESS;
	memset((uint32_t)new_proc->thread->kernel_stack-KERNEL_STACK_PER_PROCESS,0,KERNEL_STACK_PER_PROCESS);
	set_page_directory(cpdir,0);
	
	new_proc->group = kmalloc(sizeof(thread_group_t));
	memset(new_proc->group,0,sizeof(thread_group_t));
	new_proc->group->queue = kmalloc(sizeof(proc_t*));
	new_proc->group->processes = kmalloc(sizeof(proc_t*));
	new_proc->group->processes[0] = new_proc;
	new_proc->group->group_size++;
	new_proc->group->queue[0] = 0;
	new_proc->group->priority = PROC_PRIORITY_LOW;
	add_group(new_proc->group);
	
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
	new_proc->sig_stack_esp = -1;
	
	new_proc->uid = parent?parent->uid:PROC_ROOT_UID;
	
	if(parent){
		new_proc->work_dir = parent->work_dir;
		memset(new_proc->work_dir_abs,0,256);
		strcpy(new_proc->work_dir_abs,parent->work_dir_abs);
	}else{
		new_proc->work_dir = kseek("/");
		memset(new_proc->work_dir_abs,0,256);
		strcpy(new_proc->work_dir_abs,"/"); //TODO this must be per-thread I think
	}
	
	strcpy(new_proc->name,name);
	new_proc->heap = USER_HEAP;
	
	set_page_directory(new_proc->pdir,0);
	kralloc(USER_HEAP,USER_HEAP+USER_HEAP_SIZE);
	
	new_proc->heap_size = USER_HEAP_SIZE;
	kralloc(USER_STACK,USER_STACK+USER_STACK_PER_PROCESS);
	
	kralloc(SHARED_MEMORY_START,SHARED_MEMORY_END);
	
	if(current_process){
		set_page_directory(current_process->pdir,0);
	}
	
	new_proc->thread->state->useresp = USER_STACK + USER_STACK_PER_PROCESS;
	new_proc->thread->state->ebp = new_proc->thread->state->useresp;
	new_proc->thread->state->eip = 0;
	
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
	
	kinfo("Process created: '%s' with pid %d (stack %p)\n",new_proc->name,new_proc->pid,new_proc->thread->kernel_stack);

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
	set_page_directory(proc->pdir,0);
	tss_set_kernel_stack(proc->thread->kernel_stack);
	
	kinfo("Allocating space for %d cmd arguments and %d envp entries\n",argc,envsize);
	
	char** usr_argv = 0;
	char** usr_envp = 0;
	mem_t* allocation = 0;
	//Here we manually manage memory blocks in user heap
	if(argc){
		allocation = USER_HEAP;
		allocation->guard = KHEAP_GUARD_VALUE;
		allocation->size =  sizeof(char*)*argc;
		allocation->prev = 0xAABBCCDD;
		allocation->next = 0xAABBCCDD;
		usr_argv = (char**)((uint32_t)allocation + sizeof(mem_t));
		for(uint32_t i=0;i<argc;i++){
			
			allocation = (mem_t*)((uint32_t)allocation + sizeof(mem_t) + allocation->size);
			allocation->guard = KHEAP_GUARD_VALUE;
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
		allocation->guard = KHEAP_GUARD_VALUE;
		allocation->size =  sizeof(char*)*envsize;
		allocation->prev = 0xAABBCCDD;
		allocation->next = 0xAABBCCDD;
		usr_envp = (char**)((uint32_t)allocation + sizeof(mem_t));
		for(uint32_t i=0;i<envsize;i++){
			allocation = (mem_t*)((uint32_t)allocation + sizeof(mem_t) + allocation->size);
			allocation->guard = KHEAP_GUARD_VALUE;
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
	proc->thread->state->eip = entry;
	kinfo("ENTRY: %p\n",entry);

	PUSH(proc->thread->state->useresp,char**,usr_envp);
	PUSH(proc->thread->state->useresp,char**,usr_argv);
	PUSH(proc->thread->state->useresp,int,   argc);
	
	if(!init){ //This is hack, TODO rewrite 
		set_page_directory(current_process->pdir,0);
		tss_set_kernel_stack(current_process->thread->kernel_stack);
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

	group_remove(proc);
	//kffree(processes[proc->pid]->state->cr3);
	kvfree(processes[proc->pid]->thread->kernel_stack);
	kfree(processes[proc->pid]->thread->state);
	
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
			//kinfo("Sleep time left: %d",sleep_procs[0]->sleep_time);
			sleep_procs[0]->sleep_time--;
			if(sleep_procs[0]->sleep_time <= 0){
				process_awake(sleep_procs[0]);
			}else{
				break;
			}
		}
		if(!reg && current_process && validate(current_process->thread->syscall_state)){
			reg = current_process->thread->syscall_state;
			//kinfo("Restored state from syscall\n");
		}else if(!reg){
			//kinfo("No valid registers - skipping tick\n");
			return;
		}
		
		if(current_process && save){
			if(current_process->in_sig){
				save_ctx(current_process->signal_state,reg);
			}else{
				save_ctx(current_process->thread->state,reg);
			}
		}
		
		proc_t* old = current_process;
		current_process = get_ready();
		if(!current_process){
			current_process = old;
		}

		if(current_process){
			if(current_process->in_sig){
				setup_ctx(current_process->signal_state,reg,current_process->pdir,current_process->thread->kernel_stack);
			}else{
				setup_ctx(current_process->thread->state,reg,current_process->pdir,current_process->thread->kernel_stack);
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
				current_process->thread->syscall_state = reg;
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
		schedule(proc->thread->syscall_state,0);
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
		schedule(proc->thread->syscall_state,0);
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
			if(validate(proc->fswait_nodes[i])){
				pipe_add_waiter(proc->fswait_nodes[i],proc); //TODO select_fs
			}
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
				process->thread->state->eax = i; //Return which node awoken us
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
	//kinfo("Awaking %d\n",proc->pid);
	sleep_remove(proc);
	ready_insert(proc);
}

void process_create_thread(proc_t* parent,uint32_t entry){
	uint32_t pid = free_pid();
	
	if(!pid){
		crash_info_t crash;
		crash.description = "Failed to allocate pid";
		kpanic(crash);
	}
	
	if(!validate(entry)){
			kwarn("Tried to create thread for %s with invalid entry %p\n",parent->name,entry);
			return;
	}
	
	proc_t* new = kmalloc(sizeof(proc_t));
	memcpy(new,parent,sizeof(proc_t));
	new->pid = pid;
	new->thread = kmalloc(sizeof(thread_t));
	memset(new->thread,0,sizeof(thread_t)); //TODO calloc for setting memory to zero on allocation
	new->group = parent->group;
	group_insert(new);
	set_page_directory(kernel_page_directory,0);
	new->pdir = copy_page_directory(parent->pdir);
	set_page_directory(new->pdir,0);
	kralloc(USER_STACK,USER_STACK+USER_STACK_PER_PROCESS);
	set_page_directory(current_process->pdir,0);
	//kinfo("In %p: %p",current_process->pdir,virtual2physical(USER_STACK));
	new->thread->state = kmalloc(sizeof(struct registers));
	memset(new->thread->state,0,sizeof(struct registers));
	
	new->thread->state->useresp = USER_STACK + USER_STACK_PER_PROCESS;
	new->thread->state->ebp = new->thread->state->useresp;
	new->thread->state->eip = entry;
	new->thread->syscall_state = 0;
	new->thread->kernel_stack = (uint32_t)kvalloc(KERNEL_STACK_PER_PROCESS,4096) + KERNEL_STACK_PER_PROCESS;
	memset((uint32_t)new->thread->kernel_stack-KERNEL_STACK_PER_PROCESS,0,KERNEL_STACK_PER_PROCESS);

	processes[pid] = new;
	total_prcs++;
	ready_insert(new);
	
	kinfo("Thread created in %s with entry %p - %d\n",parent->name,entry,pid);
}

uint32_t process_create_shared(proc_t* proc,uint32_t buffer_size){
	if(!proc->shmem_size){
		proc->shmem_blocks = kmalloc(sizeof(shmem_block_t*));
	}else{
		proc->shmem_blocks = krealloc(proc->shmem_blocks,sizeof(shmem_block_t*)*(proc->shmem_size+1));
	}
	proc->shmem_blocks[proc->shmem_size] = kmalloc(sizeof(shmem_block_t));
	shmem_block_t* block = proc->shmem_blocks[proc->shmem_size];
	proc->shmem_size++;
	block->size = buffer_size;
	block->offset = proc->shmem_bytes;
	proc->shmem_bytes += buffer_size;
	return proc->shmem_size-1;
}
shmem_block_t* process_get_shared(proc_t* proc,uint32_t id){
	if(id < proc->shmem_size){
		return proc->shmem_blocks[id];
	}
	return 0;
}
void process_reset_shmem(proc_t* proc){
	for(uint32_t i=0;i<proc->shmem_size;i++){
		kfree(proc->shmem_blocks[i]);
	}
	kfree(proc->shmem_blocks);
	proc->shmem_size = 0;
	kralloc(SHARED_MEMORY_START,SHARED_MEMORY_END);
}

uint32_t process_open_shmem(proc_t* proc,proc_t* target,uint32_t id){
	shmem_block_t* bl = process_get_shared(target,id);
	uint32_t id1 = process_create_shared(proc,bl->size);
	shmem_block_t* own_bl = process_get_shared(proc,id1);
	copy_region(target->pdir,proc->pdir,SHARED_MEMORY_START+bl->offset,bl->size,SHARED_MEMORY_START+own_bl->offset);
	return id1;
}

