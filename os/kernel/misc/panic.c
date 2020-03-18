/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/
#include <kernel/memory/memory.h>
#include <kernel/misc/panic.h>
#include <kernel/misc/log.h>

extern tss_entry_t tss_entry;

void kpanic(crash_info_t crash){
	asm("cli");
	kerr("------------------KERNEL PANIC------------------------\n");
	
	kerr("Kernel built for arch %s\n",ARCH);
	kerr("Unhandled exception occured\n");
	kerr("Description: %s\n",crash.description?crash.description:"No desc.");
	kerr("------------------REGISTERS DUMP----------------------\n");
	if(crash.regs){
		regs_t regs = crash.regs;
		kerr("EDI=%a | ESI=%a | EBP=%a\n EBX=%a | EDX=%a | ECX=%a | EAX=%a\n",regs->edi,regs->esi,regs->ebp,regs->ebx,regs->edx,regs->ecx,regs->eax);
		kerr("GS=%a |  FS=%a |  ES=%a | DS=%a\n",regs->gs,regs->fs,regs->es,regs->ds);		
		kerr("EIP=%a | ESP=%a\n",regs->eip,regs->esp);	
		kerr("TSS[SS:ESP0]: %a %a\n",tss_entry.ss0,tss_entry.esp0);
	}else{
		kerr("Unavailable\n");
	}
	kerr("------------------EXTRA INFO---------------------------\n");
	if(crash.extra_info){
		kerr("%s\n",crash.extra_info);
	}else{
		kerr("Unavailable\n");
	}
	asm("hlt");
}
