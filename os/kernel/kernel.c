/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/
#include <kernel/multiboot.h>
#include <kernel/global.h>
#include <kernel/memory/memory.h>
#include <kernel/arch.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/tar.h>
#include <kernel/misc/debug.h>
#include <kernel/module/symtable.h>
#include <kernel/dev/acpica/acpi.h>

#include <stdio.h>
#include <stdlib.h>
//TODO Make all thread-safe

extern uint32_t* k_frame_stack;  

void init_acpi(){
	kinfo("Started ACPI initialization\n");
	
	if(AcpiInitializeTables(0,0,0) != AE_OK){
		kerr("Failed to initialize acpi tables!\n");
		while(1){
			asm("cli");
			asm("hlt");
		}
	}else{
		kinfo("ACPI tables initialized\n");
	}
	
	if(AcpiInitializeSubsystem() != AE_OK){
		kerr("Failed to initialize acpi subsystem!\n");
		while(1){
			asm("cli");
			asm("hlt");
		}
	}else{
		kinfo("ACPI subsystem initialized\n");
	}
	
	if(AcpiLoadTables() != AE_OK){
		kerr("Failed to load acpi tables!\n");
		while(1){
			asm("cli");
			asm("hlt");
		}
	}else{
		kinfo("ACPI tables loaded\n");
	}
	
	if(AcpiEnableSubsystem(ACPI_FULL_INITIALIZATION) != AE_OK){
		kerr("Failed to enable acpi subsystem!\n");
		while(1){
			asm("cli");
			asm("hlt");
		}
	}else{
		kinfo("ACPI subsystem enabled\n");
	}
	if(AcpiInitializeObjects(ACPI_FULL_INITIALIZATION)!= AE_OK){
		kerr("Failed to initialize acpi objects!\n");
		while(1){
			asm("cli");
			asm("hlt");
		}
	}else{
		kinfo("ACPI objects initialized\n");
	}
	AcpiEnable();
	kinfo("ACPI initialized\n");
}

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
	init_pipe();
	init_signals();
	init_acpi();
	
	modules_load();
	
	kmount("/","/dev/sda1",ktypeid("fat"));
	fs_node_t* node = kopen("/CHECK");
	
	
	 
	if(node){
		char buffer[62];
		kread(node,0,62,buffer);
		kinfo("CHECK TEXT: %s\n",buffer);
		kclose(node);
	}
	
	

	fs_node_t* dir = kopen("/bin/modules");
	fs_dir_t* modd = 0;
	if(dir){
		modd = kreaddir(dir);
		kclose(dir);
	}
	
	if(modd){
		kinfo("Disk module count: %d\n",modd->chld_cnt - 2);
		for(uint32_t i = 2;i < modd->chld_cnt; i++){
			kinfo("Trying to load from disk: %s\n",modd->chlds[i]->name);
			module_load(modd->chlds[i]);
		}
	}
	
	mem_stat();
	
	kcreate("/proc",VFS_TYPE_VIRTUAL);
	
	fs_node_t* init = kopen("/usr/bin/init.smp");
	if(init){
		execute(init,0,0,1);
	}
	
	asm("cli");
	for(;;) {
		asm("hlt");
    }
}
