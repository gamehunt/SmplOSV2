/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/



#include <kernel/multiboot.h>
#include <kernel/global.h>
#include <stdio.h>
#include <stdlib.h>
#include <kernel/memory/memory.h>
#include <kernel/arch.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/tar.h>
#include <kernel/misc/debug.h>
#include <kernel/module/symtable.h>
//TODO Make all thread-safe

extern uint32_t* k_frame_stack;  

void kernel_main(multiboot_info_t *mbt,uint32_t magic){
	eld_init();	
	if(magic != 0x2BADB002){
		char message[60];
		sprintf(message,"%a passed when %a expected",magic,0x2BADB002);
		crash_info_t crash;
		crash.description = "Failed to verify multiboot";
		crash.extra_info = message;
		kpanic(crash);
	}
	printf("Kernel entry\n");
	if(mbt->flags & MULTIBOOT_INFO_MODS){
		  int n = mbt->mods_count;
		  multiboot_module_t* mod = mbt->mods_addr;
		  kinfo("Found %d boot modules (base=%a)\n",n,mbt->mods_addr);
		  for (int i=0;i<n;i++){
			  module_ramdisk_add(mod);
			  if(mod->mod_end > (uint32_t)k_frame_stack){
				  k_frame_stack = (uint32_t*)mod->mod_end;
			  }
		  }
	}
	gdt_install();
	remap_PIC(0x20,0x28);
	idt_install();	
	
	init_syscalls();
	init_pmm(mbt);
	init_paging();
	init_kheap();
	init_symtable();
  	init_pci();
	init_pit();
	init_rtc();
	init_vfs(); 
	
	modules_load();
	
	
	//while(1);
	kinfo("CHCK: %x\n",((uint32_t(*)())((sym_entry_t*)symbol_seek("__exported")->addr))());
	
	//Below this point is multiproc.
	uint8_t buffer[512];
	kread("/dev/sda",0,1,buffer);
	kinfo("First 3 bytes of /dev/sda: %a %a %a\n",buffer[0],buffer[1],buffer[2]);
	
	kmount("/root","/dev/sda1",3);
	fs_node_t* node = kseek("/root/CHECK");
	
	
	 
	if(node){
		char buffer[62];
		knread(node,0,62,buffer);
		kinfo("CHECK TEXT: %s\n",buffer);
	}
	
	fs_dirent_t* modd = kreaddir("/root/bin/modules");
	
	kinfo("Disk module count: %d\n",modd->chld_cnt);
	if(modd){
		for(uint32_t i = 2;i < modd->chld_cnt; i++){
			kinfo("Trying to load from disk: %s\n",modd->chlds[i]->name);
			module_load(modd->chlds[i]);
		}
	}
	
	mem_stat();

	create_process(kseek("/root/usr/bin/init.smp"));

	
	for(;;) {
		asm("hlt");
    }
}
