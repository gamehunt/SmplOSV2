/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/interrupts/syscalls.h>
#include <kernel/interrupts/isr.h>
#include <kernel/fs/vfs.h>
#include <kernel/proc/proc.h>

#define MAX_SYSCALL 128

typedef uint32_t(* syscall_t)(uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e);

static syscall_t syscalls[MAX_SYSCALL];

void syscall_handler(regs_t r){
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
	

	//kinfo("Syscall exit\n");
}

void register_syscall(uint16_t id,syscall_t handler){
	syscalls[id] = handler;
}

uint32_t sys_echo(uint32_t str,uint32_t a,uint32_t b,uint32_t c,uint32_t d){
	kinfo("[SYS_ECHO] %s %d %d %d %d\n",str,a,b,c,d);
	return 0;
}

uint32_t sys_read(uint32_t fd,uint32_t offs_high,uint32_t offs_low,uint32_t size,uint32_t buffer){
	if(get_current_process()->f_descs_cnt <= fd){
		return 0;
	}
	fs_node_t* node = get_current_process()->f_descs[fd];
	uint64_t offs = (uint64_t)offs_high << 32 | offs_low;
	return kread((fs_node_t*)node,offs,size,(uint8_t*)buffer);
}

uint32_t sys_write(uint32_t fd,uint32_t offs_high,uint32_t offs_low,uint32_t size,uint32_t buffer){
	if(get_current_process()->f_descs_cnt <= fd){
		return 0;
	}
	fs_node_t* node = get_current_process()->f_descs[fd];
	uint64_t offs = (uint64_t)offs_high << 32 | offs_low;
	//kinfo("[SYS_WRITE] %s %e %d %a\n",path,offs,size,buffer);
	return kwrite((fs_node_t*)node,offs,size,(uint8_t*)buffer);
}

uint32_t sys_open(uint32_t path,uint32_t _,uint32_t __,uint32_t ___,uint32_t _____){
	//kinfo("[SYS_SEEK] %s\n",(char*)path);
	fs_node_t* node = kopen((char*)path);
	get_current_process()->f_descs_cnt++;
	if(get_current_process()->f_descs){
		get_current_process()->f_descs = krealloc(get_current_process()->f_descs,get_current_process()->f_descs_cnt*sizeof(fs_node_t*));	
	}else{
		get_current_process()->f_descs_cnt = 1;
		get_current_process()->f_descs = kmalloc(sizeof(fs_node_t*));
	}
	
	get_current_process()->f_descs[get_current_process()->f_descs_cnt-1] = node;
	
	return get_current_process()->f_descs_cnt - 1;
}

uint32_t sys_readdir(uint32_t fd,uint32_t _,uint32_t __,uint32_t ___,uint32_t _____){
	if(get_current_process()->f_descs_cnt <= fd){
		return 0;
	}
	fs_node_t* node = get_current_process()->f_descs[fd];
	return (uint32_t)kreaddir(node);
}

uint32_t sys_exec(uint32_t path,uint32_t argc,uint32_t argv,uint32_t ___,uint32_t _____){
	return execute(kseek((char*)path),0);
}

uint32_t sys_ioctl(uint32_t fd,uint32_t req,uint32_t argp,uint32_t ___,uint32_t _____){
	if(get_current_process()->f_descs_cnt <= fd){
		return 0;
	}
	fs_node_t* node = get_current_process()->f_descs[fd];
	return kioctl(node,req,argp);
}

uint32_t sys_exit(uint32_t code,uint32_t _,uint32_t __,uint32_t ___,uint32_t _____){
	exit(get_current_process());
	return 0;
}


uint32_t sys_fswait(uint32_t fds,uint32_t cnt,uint32_t __,uint32_t ___,uint32_t _____){
	uint32_t* fds_ptr = (uint32_t*)fds;
	fs_node_t** nodes = kmalloc(sizeof(fs_node_t*)*cnt);
	for(uint32_t i=0;i<cnt;i++){
		nodes[i] = get_current_process()->f_descs[fds_ptr[i]];
	}
	process_fswait(get_current_process(),nodes,cnt);
	return 0;
}

uint32_t sys_yield(uint32_t _,uint32_t __,uint32_t ___,uint32_t ____,uint32_t _____){
	schedule(0,1);
	return 0;
}

uint32_t sys_fork(uint32_t _,uint32_t __,uint32_t ___,uint32_t ____,uint32_t _____){
	return fork();
}

void init_syscalls(){
	isr_set_handler(127,&syscall_handler);
	memset(syscalls,0,sizeof(syscall_t)*MAX_SYSCALL);
	
	register_syscall(0,&sys_echo);
	register_syscall(1,&sys_read);
	register_syscall(2,&sys_write);
	register_syscall(3,&sys_open);
	register_syscall(4,&sys_readdir);
	register_syscall(5,&sys_exec);
	register_syscall(6,&sys_ioctl);
	register_syscall(7,&sys_exit);
	register_syscall(8,&sys_fswait);
	register_syscall(9,&sys_yield);
	register_syscall(10,&sys_fork);
}
