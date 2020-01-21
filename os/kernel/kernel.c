#include <kernel/multiboot.h>
#include <kernel/global.h>
#include <stdio.h>
#include <stdlib.h>
//#include <kernel/debug/debug.h>
#include <kernel/memory/memory.h>
#include <kernel/arch.h>
#include <kernel/fs/vfs.h>
#include <kernel/module/symbol.h>

//TODO Make all shit thread-safe
//TODO InitRD
//TODO Module loading (ELF)

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

	gdt_install();
	remap_PIC(0x20,0x28);
	idt_install();	
	init_pmm(mbt);
	init_paging();
	init_kheap();
  	init_pci();
	init_pit();
	init_rtc();
	init_vfs(); 
	init_tty();
	
	//Below this point is multiproc.
	init_sched();
	
	for(;;) {
		asm("hlt");
    }
}

EXPORT_SYMBOL(kernel_main)
