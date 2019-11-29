#include <kernel/multiboot.h>
#include <kernel/global.h>
#include <stdio.h>
#include <stdlib.h>
//#include <kernel/debug/debug.h>
#include <kernel/memory/memory.h>
#include <kernel/arch.h>
#include <kernel/fs/vfs.h>


//TODO make all shit thread-safe
void kernel_main(multiboot_info_t *mbt,uint32_t magic){
	//mbt = (uint32_t*)((uint32_t)mbt + 0xC0000000);
        //while(1);
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
	
	//Below this point is multithreading
	init_sched();
	for(;;) {
		asm("hlt");
    }
}
