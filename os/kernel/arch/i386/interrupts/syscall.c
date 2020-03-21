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
	//kinfo("Syscall eneter\n");
	if(r->eax > MAX_SYSCALL){
		kerr("Invalid syscall: %a\n",r->eax);
		return;
	}
	
	syscall_t sysc = syscalls[r->eax];
	if(sysc){
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

uint32_t sys_read(uint32_t node,uint32_t offs_high,uint32_t offs_low,uint32_t size,uint32_t buffer){
	uint64_t offs = (uint64_t)offs_high << 32 | offs_low;
	return kread((fs_node_t*)node,offs,size,(uint8_t*)buffer);
}

uint32_t sys_write(uint32_t node,uint32_t offs_high,uint32_t offs_low,uint32_t size,uint32_t buffer){
	uint64_t offs = (uint64_t)offs_high << 32 | offs_low;
	//kinfo("[SYS_WRITE] %s %e %d %a\n",path,offs,size,buffer);
	return kwrite((fs_node_t*)node,offs,size,(uint8_t*)buffer);
}

uint32_t sys_open(uint32_t path,uint32_t _,uint32_t __,uint32_t ___,uint32_t _____){
	//kinfo("[SYS_SEEK] %s\n",(char*)path);
	return (uint32_t)kopen((char*)path);
}

uint32_t sys_readdir(uint32_t node,uint32_t _,uint32_t __,uint32_t ___,uint32_t _____){
	return (uint32_t)kreaddir((fs_node_t*)node);
}

uint32_t sys_exec(uint32_t path,uint32_t argc,uint32_t argv,uint32_t ___,uint32_t _____){
	return create_process(kseek((char*)path));
}

uint32_t sys_ioctl(uint32_t node,uint32_t req,uint32_t argp,uint32_t ___,uint32_t _____){
	return kioctl(node,req,argp);
}

uint32_t sys_exit(uint32_t code,uint32_t _,uint32_t __,uint32_t ___,uint32_t _____){
	exit(get_current_process());
	return 0;
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
}
