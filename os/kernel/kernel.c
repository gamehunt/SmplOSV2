#include <kernel/multiboot.h>
#include <kernel/global.h>
#include <stdio.h>
#include <stdlib.h>
//#include <kernel/debug/debug.h>
#include <kernel/memory/memory.h>
#include <kernel/arch.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/tar.h>

//TODO Make all shit thread-safe
//TODO InitRD
//TODO Module loading (ELF)

extern fs_node_t* root;

extern uint32_t* k_frame_stack;  
void kernel_main(multiboot_info_t *mbt,uint32_t magic){
	terminal_init();	
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
		  //kinfo("%s\n",mod->mod_start);
		  for (int i=0;i<n;i++){
			  add_ramdisk_module(mod);
			  //kinfo("%a %a\n",(uint32_t)k_frame_stack,mod->mod_end);
			  if(mod->mod_end > (uint32_t)k_frame_stack){
				  k_frame_stack = (uint32_t*)mod->mod_end;
				 //kinfo("HERE\n");
			  }
		  }
	}
	//while(1);
	gdt_install();
	remap_PIC(0x20,0x28);
	
	idt_install();	
	
	init_pmm(mbt);
	init_paging();
	init_kheap();
	init_symtable();
  	init_pci();
	init_pit();
	init_rtc();
	init_vfs(); 
	create_vfs_mapping("/huy");
	init_tty();

	ramdisk_load();

	//Below this point is multiproc.
	init_sched();
	
	for(;;) {
		asm("hlt");
    }
}
