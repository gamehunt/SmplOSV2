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

#define MAX_SYSCALL 128


typedef uint32_t(* syscall_t)(uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e);

static syscall_t syscalls[MAX_SYSCALL];

void syscall_handler(regs_t r){
	
	lock_interrupts();
	
	if(r->eax > MAX_SYSCALL){
		kerr("Invalid syscall: %a\n",r->eax);
		return;
	}
	
	syscall_t sysc = syscalls[r->eax];
	if(sysc){
		get_current_process()->syscall_state = r;
		//kinfo("Syscall %d -> %a %a %a %a %a\n",r->eax,r->ebx,r->ecx,r->edx,r->esi,r->edi);
		uint32_t ret = sysc(r->ebx,r->ecx,r->edx,r->esi,r->edi);
		r->eax = ret;
	}else{
		kerr("Syscall %a has null handler\n",r->eax);
	}
	
	unlock_interrupts();

	//kinfo("Syscall exit\n");
}

void register_syscall(uint16_t id,syscall_t handler){
	syscalls[id] = handler;
}

uint32_t sys_echo(uint32_t str,uint32_t a,uint32_t b,uint32_t c,uint32_t d){
	kinfo("[SYS_ECHO][%d] %s %d %d %d %d\n",get_current_process()->pid,str,a,b,c,d);
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
	//kinfo("[SYS_WRITE] %s(%d): %d %e %d %a\n",node->name,node->fsid,fd,offs,size,buffer);
	return kwrite((fs_node_t*)node,offs,size,(uint8_t*)buffer);
}

uint32_t sys_open(uint32_t path,uint32_t flags,uint32_t __,uint32_t ___,uint32_t _____){
	
	fs_node_t* node = kopen((char*)path);
	if(!node && (flags & F_CREATE)){
		node = kcreate(path,0);
	}else if(!node){
		return -1;
	}
	node->open_flags = flags;
	for(uint32_t i=0;i<get_current_process()->f_descs_cnt;i++){
		if(!get_current_process()->f_descs[i]){
			get_current_process()->f_descs[i] = node;
		//	kinfo("Opened %s as %d\n",path,i);
			return i;
		}
	}
	get_current_process()->f_descs_cnt++;
	if(get_current_process()->f_descs){
		get_current_process()->f_descs = krealloc(get_current_process()->f_descs,get_current_process()->f_descs_cnt*sizeof(fs_node_t*));	
	}else{
		get_current_process()->f_descs = kmalloc(sizeof(fs_node_t*));
	}
	
	
	
	get_current_process()->f_descs[get_current_process()->f_descs_cnt-1] = node;
	//kinfo("[SYS_OPEN] %s(%d) - %d\n",(char*)path,node->fsid,get_current_process()->f_descs_cnt-1);
	return get_current_process()->f_descs_cnt - 1;
}


uint32_t sys_close(uint32_t fds,uint32_t _,uint32_t __,uint32_t ___,uint32_t _____){
//	kinfo("[SYS_CLOSE] %d\n",fds);
	proc_t* proc = get_current_process();
	if(proc->f_descs_cnt > fds && proc->f_descs[fds]){
		kclose(proc->f_descs[fds]);
		proc->f_descs[fds] = 0;
	}
	return 0;
}

uint32_t sys_readdir(uint32_t fd,uint32_t index,uint32_t ptr,uint32_t ___,uint32_t _____){
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

uint32_t sys_exec(uint32_t path,uint32_t argv,uint32_t envp,uint32_t _,uint32_t _____){
	//kinfo("Trying to execute: %s\n",path);
	proc_t* p = execute(kseek((char*)path),argv,envp,0);
	if(!p){
		return 0;
	}
	return p->pid;
}

uint32_t sys_clone(uint32_t _,uint32_t __,uint32_t ___,uint32_t ____,uint32_t _____){
	create_process("",get_current_process(),1);
	return 0;
}

uint32_t sys_ioctl(uint32_t fd,uint32_t req,uint32_t argp,uint32_t ___,uint32_t _____){
	if(get_current_process()->f_descs_cnt <= fd){
		return 0;
	}
	fs_node_t* node = get_current_process()->f_descs[fd];
	return kioctl(node,req,argp);
}

uint32_t sys_exit(uint32_t code,uint32_t _,uint32_t __,uint32_t ___,uint32_t _____){
	proc_exit(get_current_process());
	return 0;
}


uint32_t sys_fswait(uint32_t fds,uint32_t cnt,uint32_t __,uint32_t ___,uint32_t _____){
	uint32_t* fds_ptr = (uint32_t*)fds;
	fs_node_t** nodes = kmalloc(sizeof(fs_node_t*)*cnt);
	for(uint32_t i=0;i<cnt;i++){
		if(fds_ptr[i] < get_current_process()->f_descs_cnt){
			nodes[i] = get_current_process()->f_descs[fds_ptr[i]];
		}
	}
	process_fswait(get_current_process(),nodes,cnt);
	return 0;
}

uint32_t sys_yield(uint32_t _,uint32_t __,uint32_t ___,uint32_t ____,uint32_t _____){
	schedule(0,1);
	return 0;
}

uint32_t sys_sbrk(uint32_t size,uint32_t __,uint32_t ___,uint32_t ____,uint32_t _____){
	proc_t* proc = get_current_process();
	
	for(uint32_t i=0;i<size;i+=4096){
		knpalloc((uint32_t)proc->heap + proc->heap_size);
		proc->heap_size += 4096;
	}
	
	return proc->heap;
}

uint32_t sys_assign(uint32_t fd_dest,uint32_t fd_src,uint32_t ___,uint32_t ____,uint32_t _____){
	sys_close(fd_dest,0,0,0,0);
	proc_t* proc = get_current_process();
	fs_node_t* node = kmalloc(sizeof(fs_node_t));
	memcpy(node,proc->f_descs[fd_src],sizeof(fs_node_t));
	proc->f_descs[fd_dest] = node;
	sys_close(fd_src,0,0,0,0);
}

uint32_t sys_sig(uint32_t pid,uint32_t sig,uint32_t ___,uint32_t ____,uint32_t _____){
	send_signal(get_process_by_pid(pid),sig);
	return 0;
}

uint32_t sys_sighandl(uint32_t sig,uint32_t handler,uint32_t ___,uint32_t ____,uint32_t _____){
	set_sig_handler(get_current_process(),handler,sig);
	return 0;
}

uint32_t sys_sigexit(uint32_t __,uint32_t _,uint32_t ___,uint32_t ____,uint32_t _____){
	exit_sig(get_current_process());
	return 0;
}

uint32_t sys_time(uint32_t __,uint32_t _,uint32_t ___,uint32_t ____,uint32_t _____){
	return rtc_current_time(); //Returns wrong values. We dont count extra day each fourth year!
}
uint32_t sys_waitpid(uint32_t pid,uint32_t _,uint32_t ___,uint32_t ____,uint32_t _____){
	process_waitpid(get_current_process(),pid); 
	return 0;
}

uint32_t sys_getcwd(uint32_t buffer,uint32_t buffer_size,uint32_t ___,uint32_t ____,uint32_t _____){
	if(buffer_size < strlen(get_current_process()->work_dir_abs)){
	//	kerr("Buffer size too small: need %d\n",strlen(get_current_process()->work_dir_abs));
		return 0;
	}
	strcpy(buffer,get_current_process()->work_dir_abs);
	return buffer;
}

uint32_t sys_chdir(uint32_t path,uint32_t _,uint32_t ___,uint32_t ____,uint32_t _____){
	fs_node_t* node = kseek(path);
	if(node){
		strcpy(get_current_process()->work_dir_abs,canonize_absolute(path));
		get_current_process()->work_dir = node;
		return 0;
	}
	return -1;
}
uint32_t sys_getpid(uint32_t __,uint32_t _,uint32_t ___,uint32_t ____,uint32_t _____){
	return get_current_process()->pid;
}
uint32_t sys_getuid(uint32_t __,uint32_t _,uint32_t ___,uint32_t ____,uint32_t _____){
	return get_current_process()->uid;
}
uint32_t sys_setuid(uint32_t uid,uint32_t _,uint32_t ___,uint32_t ____,uint32_t _____){
	if(get_current_process()->uid != PROC_ROOT_UID){
		return -1; //Only root can change it's owner
	}
	get_current_process()->uid = uid;
	return 0;
}
uint32_t sys_link(uint32_t patha,uint32_t pathb,uint32_t ___,uint32_t ____,uint32_t _____){
	return klink(patha,pathb)?1:0;
}
uint32_t sys_sleep(uint32_t ticks,uint32_t __,uint32_t ___,uint32_t ____,uint32_t _____){
	process_sleep(get_current_process(),ticks);
	return 0;
}
uint32_t sys_getppid(uint32_t _, uint32_t __,uint32_t ___,uint32_t ____,uint32_t _____){
	if(!get_current_process()->parent){
		return -1;
	}
	return get_current_process()->parent->pid;;
}
uint32_t sys_pipe(uint32_t path, uint32_t size,uint32_t ___,uint32_t ____,uint32_t _____){
	fs_node_t* node = pipe_create(path,size);
	return node?0:1;
}
uint32_t sys_pwreq(uint32_t req, uint32_t __,uint32_t ___,uint32_t ____,uint32_t _____){
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
	register_syscall(SYS_CLONE,&sys_clone);
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
}
