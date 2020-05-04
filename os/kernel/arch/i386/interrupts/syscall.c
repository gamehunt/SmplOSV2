/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/interrupts/syscalls.h>
#include <kernel/interrupts/isr.h>
#include <kernel/fs/vfs.h>
#include <kernel/proc/proc.h>
#include <kernel/misc/pathutils.h>
#include <kernel/dev/acpica/acpi.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_SYSCALL 128


typedef uint32_t(* syscall_t)(uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e);

static syscall_t syscalls[MAX_SYSCALL];

void syscall_handler(regs_t r){
	
	lock_interrupts();
	
	if(r->eax >= MAX_SYSCALL){
		kerr("Invalid syscall: %p\n",r->eax);
		return;
	}
	//kinfo("Syscall %d\n",r->eax);
	syscall_t sysc = syscalls[r->eax];
	if(sysc){
		proc_t* cp = get_current_process();
		cp->thread->syscall_state = r;
		uint32_t ret = sysc(r->ebx,r->ecx,r->edx,r->esi,r->edi);
		if(cp == get_current_process()){
			cp->thread->syscall_state->eax = ret;
		}
	}else{
		kerr("Syscall %p has null handler\n",r->eax);
	}
	//kinfo("Syscall end\n");
	unlock_interrupts();
}

void register_syscall(uint16_t id,syscall_t handler){
	syscalls[id] = handler;
}

uint32_t sys_echo(uint32_t str,uint32_t va,uint32_t b,uint32_t c,uint32_t d){
	UNUSED(b);
	UNUSED(c);
	UNUSED(d);
	vprintf((const char*)str,va);
	return 0;
}

uint32_t sys_read(uint32_t fd,uint32_t offs_high,uint32_t offs_low,uint32_t size,uint32_t buffer){
	if(get_current_process()->f_descs_cnt <= fd){
		//kinfo("ENOFD\n");
		return 0;
	}
	fs_node_t* node = get_current_process()->f_descs[fd];
	if(!(node->open_flags & F_READ)){
		//kinfo("ENOPERM\n");
		return 0;
	}
	uint64_t offs = (uint64_t)offs_high << 32 | offs_low;
	return kread((fs_node_t*)node,offs,size,(uint8_t*)buffer);
}

uint32_t sys_write(uint32_t fd,uint32_t offs_high,uint32_t offs_low,uint32_t size,uint32_t buffer){
	if(get_current_process()->f_descs_cnt <= fd){
		return 0;
	}
	fs_node_t* node = get_current_process()->f_descs[fd];
	if(!(node->open_flags & F_WRITE)){
		return 0;
	}
	uint64_t offs = (uint64_t)offs_high << 32 | offs_low;
	//kinfo("[SYS_WRITE] %s(%d): %d %ld %d %p\n",node->name,node->fsid,fd,offs,size,buffer);
	return kwrite((fs_node_t*)node,offs,size,(uint8_t*)buffer);
}

uint32_t sys_fstat(uint32_t fd,uint32_t stat,uint32_t a,uint32_t b,uint32_t c){
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	if(get_current_process()->f_descs_cnt <= fd || !validate(stat)){
		return 1;
	}
	fs_node_t* node = get_current_process()->f_descs[fd];
	if(validate(node)){
		((stat_t*)stat)->st_size = node->fsid == 1?0:node->size;
		return 0;
	}else{
		return 1;
	}
}

uint32_t sys_open(uint32_t path,uint32_t flags,uint32_t a,uint32_t b,uint32_t c){
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	
//	kinfo("[SYS_OPEN] %s - start\n",(char*)path);
	
	fs_node_t* node = kopen((char*)path);
	if(!validate(node) && (flags & F_CREATE)){
		node = kcreate(path,0);
		if(!validate(node)){
			// kwarn("[1]Failed to open %s\n",(char*)path);
			return -1;
		}
	}else if(!validate(node)){
		//kwarn("[2]Failed to open %s\n",(char*)path);
		return -1;
	}
	node->open_flags = flags;
	for(uint32_t i=0;i<get_current_process()->f_descs_cnt;i++){
		if(!validate(get_current_process()->f_descs[i])){
			get_current_process()->f_descs[i] = node;
		//	kinfo("Opened %s as %d\n",path,i);
		
			return i;
		}
	}
	get_current_process()->f_descs_cnt++;
	if(validate(get_current_process()->f_descs)){
		get_current_process()->f_descs = krealloc(get_current_process()->f_descs,get_current_process()->f_descs_cnt*sizeof(fs_node_t*));	
	}else{
		get_current_process()->f_descs = kmalloc(sizeof(fs_node_t*));
	}
	
	
	
	get_current_process()->f_descs[get_current_process()->f_descs_cnt-1] = node;
	//kinfo("[SYS_OPEN] %s(%d) - %d\n",(char*)path,node->fsid,get_current_process()->f_descs_cnt-1);
	return get_current_process()->f_descs_cnt - 1;
}


uint32_t sys_close(uint32_t fds,uint32_t a,uint32_t b,uint32_t c,uint32_t d){
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	UNUSED(d);
	proc_t* proc = get_current_process();
	if(proc->f_descs_cnt > fds && validate(proc->f_descs[fds])){
		kclose(proc->f_descs[fds]);
		proc->f_descs[fds] = 0;
	}
	return 0;
}

uint32_t sys_readdir(uint32_t fd,uint32_t index,uint32_t ptr,uint32_t a,uint32_t b){
	UNUSED(a);
	UNUSED(b);
	if(get_current_process()->f_descs_cnt <= fd){
		return 1;
	}
	if(!validate(ptr)){
		return 1;
	}
	fs_node_t* node = get_current_process()->f_descs[fd];
	fs_dir_t* kdirent = kreaddir(node);
	if(kdirent->chld_cnt > index){
		struct dirent* dent = (struct dirent*)ptr;
		dent->d_ino = index;
		memcpy(dent->d_name,kdirent->chlds[index]->name,strlen(kdirent->chlds[index]->name));
	}else{
		kfree(kdirent);
		return 1;
	}
	kfree(kdirent);
	
	return 0;
}

uint32_t sys_exec(uint32_t path,uint32_t argv,uint32_t envp,uint32_t a,uint32_t b){
	UNUSED(a);
	UNUSED(b);
	proc_t* p = execute(kseek((char*)path),argv,envp,0);
	if(!p){
		return 0;
	}
	return p->pid;
}

uint32_t sys_ioctl(uint32_t fd,uint32_t req,uint32_t argp,uint32_t a,uint32_t b){
	UNUSED(a);
	UNUSED(b);
	if(get_current_process()->f_descs_cnt <= fd){
		return 0;
	}
	fs_node_t* node = get_current_process()->f_descs[fd];
	return kioctl(node,req,argp);
}

uint32_t sys_exit(uint32_t code,uint32_t a,uint32_t b,uint32_t c,uint32_t d){
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	UNUSED(d);
	proc_exit(get_current_process());
	return 0;
}


uint32_t sys_fswait(uint32_t fds,uint32_t cnt,uint32_t a,uint32_t b,uint32_t c){
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	
	uint32_t* fds_ptr = (uint32_t*)fds;
	fs_node_t** nodes = kmalloc(sizeof(fs_node_t*)*cnt);
	
	//kinfo("[FSWAIT] Allocd %d \n",cnt);
	uint32_t j = 0;
	for(uint32_t i=0;i<cnt;i++){
		//kinfo("Trying to add: %d(%d/%d)\n",fds_ptr[i],i,cnt);
		if(fds_ptr[i] < get_current_process()->f_descs_cnt){
			nodes[j] = get_current_process()->f_descs[fds_ptr[i]];
			j++;
		}
	}
	
	process_fswait(get_current_process(),nodes,cnt);
	return 0;
}

uint32_t sys_yield(uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e){
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	UNUSED(d);
	UNUSED(e);
	schedule(0,1);
	return 0;
}

uint32_t sys_sbrk(uint32_t size,uint32_t a,uint32_t b,uint32_t c,uint32_t d){
	
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	UNUSED(d);
	
	
	
	uint32_t frame = kfalloc();
	thread_group_t* group = get_current_process()->group;
	
	for(uint32_t i=0;i<group->group_size;i++){
		if(!group->processes[i]){
			continue;
		}
		proc_t* proc = group->processes[i];
		//kinfo("SBRK FOR %s(%d)\n",proc->name,proc->pid);
		for(uint32_t i=0;i<size;i+=4096){
			set_page_directory(proc->pdir,0);
			kmpalloc((uint32_t)proc->heap + proc->heap_size,frame,0);
			proc->heap_size += 4096;
		}
	}
	set_page_directory(get_current_process()->pdir,0);
	return get_current_process()->heap;
}

uint32_t sys_assign(uint32_t fd_dest,uint32_t fd_src,uint32_t a,uint32_t b,uint32_t c){
	
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	
	sys_close(fd_dest,0,0,0,0);
	proc_t* proc = get_current_process();
	fs_node_t* node = kmalloc(sizeof(fs_node_t));
	memcpy(node,proc->f_descs[fd_src],sizeof(fs_node_t));
	proc->f_descs[fd_dest] = node;
	sys_close(fd_src,0,0,0,0);
}

uint32_t sys_sig(uint32_t pid,uint32_t sig,uint32_t a,uint32_t b,uint32_t c){
	
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);

	send_signal(get_process_by_pid(pid),sig);
	return 0;
}

uint32_t sys_sighandl(uint32_t sig,uint32_t handler,uint32_t a,uint32_t b,uint32_t c){
	
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	
	set_sig_handler(get_current_process(),handler,sig);
	return 0;
}

uint32_t sys_sigexit(uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e){
	
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	UNUSED(d);
	UNUSED(e);
	
	exit_sig(get_current_process());
	return 0;
}

uint32_t sys_time(uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e){
	
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	UNUSED(d);
	UNUSED(e);
	
	return rtc_current_time(); //Returns wrong values. We dont count extra day each fourth year!
}
uint32_t sys_waitpid(uint32_t pid,uint32_t a,uint32_t b,uint32_t c,uint32_t d){
	
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	UNUSED(d);
	
	process_waitpid(get_current_process(),pid); 
	return 0;
}

uint32_t sys_getcwd(uint32_t buffer,uint32_t buffer_size,uint32_t a,uint32_t b,uint32_t c){
	
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	
	if(buffer_size < strlen(get_current_process()->work_dir_abs)){
	//	kerr("Buffer size too small: need %d\n",strlen(get_current_process()->work_dir_abs));
		return 0;
	}
	strcpy(buffer,get_current_process()->work_dir_abs);
	return buffer;
}

uint32_t sys_chdir(uint32_t path,uint32_t a,uint32_t b,uint32_t c,uint32_t d){
	
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	UNUSED(d);
	
	fs_node_t* node = kseek(path);
	if(node){
		strcpy(get_current_process()->work_dir_abs,canonize_absolute(path));
		get_current_process()->work_dir = node;
		return 0;
	}
	return -1;
}
uint32_t sys_getpid(uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e){
	
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	UNUSED(d);
	UNUSED(e);
	
	return get_current_process()->pid;
}
uint32_t sys_getuid(uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e){
	
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	UNUSED(d);
	UNUSED(e);
	
	return get_current_process()->uid;
}
uint32_t sys_setuid(uint32_t uid,uint32_t a,uint32_t b,uint32_t c,uint32_t d){
	
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	UNUSED(d);
	
	if(get_current_process()->uid != PROC_ROOT_UID){
		return -1; //Only root can change it's owner
	}
	get_current_process()->uid = uid;
	return 0;
}
uint32_t sys_link(uint32_t patha,uint32_t pathb,uint32_t a,uint32_t b,uint32_t c){
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	return klink(patha,pathb)?1:0;
}
uint32_t sys_sleep(uint32_t ticks,uint32_t a,uint32_t b,uint32_t c,uint32_t d){
	
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	UNUSED(d);
	
	process_sleep(get_current_process(),ticks);
	return 0;
}
uint32_t sys_getppid(uint32_t a, uint32_t b,uint32_t c,uint32_t d,uint32_t e){
	
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	UNUSED(d);
	UNUSED(e);
	
	if(!get_current_process()->parent){
		return -1;
	}
	return get_current_process()->parent->pid;;
}
uint32_t sys_pipe(uint32_t path, uint32_t size,uint32_t a,uint32_t b,uint32_t c){
	
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	
	fs_node_t* node = pipe_create(path,size);
	return node?0:1;
}
uint32_t sys_pwreq(uint32_t req, uint32_t a,uint32_t b,uint32_t c,uint32_t d){
	
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	UNUSED(d);
	
	if(req == 0){
		kinfo("Shooting down\n");
		//TODO move to separate kernel function and unload all shit in it
		AcpiEnterSleepStatePrep(5);
		asm("cli");
		AcpiEnterSleepState(5);
		while(1){
			asm("hlt");
		}
	}
	if(req == 1){
		kinfo("Rebooting\n");
		//TODO move to separate kernel function and unload all shit in it
		if(AcpiGbl_FADT.Flags & ACPI_FADT_RESET_REGISTER){
			kmpalloc(0x40000000,AcpiGbl_FADT.ResetRegister.Address,0);//Figure out where is AcpiGbl_FADT.ResetRegister.Address?
			*((uint8_t*)0x40000000) = AcpiGbl_FADT.ResetValue;
			outb(AcpiGbl_FADT.ResetRegister.Address, AcpiGbl_FADT.ResetValue);
		}else{
			//Hack code for 8042 processor
			uint8_t good = 0x02;
			while (good & 0x02)
				good = inb(0x64);
			outb(0x64, 0xFE);
		}
		while(1){
			asm("hlt");
		}
	}
}

uint32_t sys_prior(uint32_t req, uint32_t who,uint32_t which,uint32_t prior,uint32_t d){
	UNUSED(d);
	
	if(req){
		if(!get_current_process()->uid){
			ready_remove(get_current_process());
			get_current_process()->group->priority = prior;
			ready_insert(get_current_process());
		}
	}else{
		return get_current_process()->group->priority;
	}
	return 0;
}

uint32_t sys_thread(uint32_t entry, uint32_t a,uint32_t b,uint32_t c,uint32_t d){
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	UNUSED(d);
	
	process_create_thread(get_current_process(),entry);
	return 0;
}
uint32_t sys_shmem_create(uint32_t size, uint32_t a,uint32_t b,uint32_t c,uint32_t d){
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	UNUSED(d);
	
	
	return process_create_shared(get_current_process(),size);
}
uint32_t sys_shmem_open(uint32_t pid, uint32_t sh_id,uint32_t bl,uint32_t c,uint32_t d){
	
	UNUSED(c);
	UNUSED(d);
	
	shmem_block_t* b = bl;
	 
	memcpy(b,process_open_shmem(get_current_process(),get_process_by_pid(pid),sh_id),sizeof(shmem_block_t));
	
	return 0;
}
uint32_t sys_shmem_reset(uint32_t e, uint32_t a,uint32_t b,uint32_t c,uint32_t d){
	UNUSED(a);
	UNUSED(b);
	UNUSED(c);
	UNUSED(d);
	UNUSED(e);
	process_reset_shmem(get_current_process());
	return 0;
}

void init_syscalls(){
	isr_set_handler(127,&syscall_handler);
	memset(syscalls,0,sizeof(syscall_t)*MAX_SYSCALL);
	
	register_syscall(SYS_ECHO,&sys_echo);
	register_syscall(SYS_READ,&sys_read);
	register_syscall(SYS_WRITE,&sys_write);
	register_syscall(SYS_OPEN,&sys_open);
	register_syscall(SYS_READDIR,&sys_readdir);
	register_syscall(SYS_EXEC,&sys_exec);
	register_syscall(SYS_IOCTL,&sys_ioctl);
	register_syscall(SYS_EXIT,&sys_exit);
	register_syscall(SYS_FSWAIT,&sys_fswait);
	register_syscall(SYS_YIELD,&sys_yield);
	register_syscall(SYS_CLOSE,&sys_close);
	register_syscall(SYS_SBRK,&sys_sbrk);
	register_syscall(SYS_ASSIGN,&sys_assign);
	register_syscall(SYS_SIG,&sys_sig);
	register_syscall(SYS_SIGHANDL,&sys_sighandl);
	register_syscall(SYS_SIGEXIT,&sys_sigexit);
	register_syscall(SYS_TIME,&sys_time);
	register_syscall(SYS_WAITPID,&sys_waitpid);
	register_syscall(SYS_GETCWD,&sys_getcwd);
	register_syscall(SYS_CHDIR,&sys_chdir);
	register_syscall(SYS_GETUID,&sys_getuid);
	register_syscall(SYS_SETUID,&sys_setuid);
	register_syscall(SYS_GETPID,&sys_getpid);
	register_syscall(SYS_LINK,&sys_link);
	register_syscall(SYS_SLEEP,&sys_sleep);
	register_syscall(SYS_GETPPID,&sys_getppid);
	register_syscall(SYS_PIPE,&sys_pipe);
	register_syscall(SYS_PWREQ,&sys_pwreq);
	register_syscall(SYS_FSSTAT,&sys_fstat);
	register_syscall(SYS_PRIOR,&sys_prior);
	register_syscall(SYS_THREAD,&sys_thread);
	register_syscall(SYS_SHMEM_CREATE,&sys_shmem_create);
	register_syscall(SYS_SHMEM_OPEN,&sys_shmem_open);
	register_syscall(SYS_SHMEM_RESET,&sys_shmem_reset);
}
