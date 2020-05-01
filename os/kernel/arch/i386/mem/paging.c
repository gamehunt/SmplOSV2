
/*	
    Copyright (C) 2020
     
    Author: gamehunt 
*/

//PDE
//31[********]11[***]9[*********]0
//  [pte_addr]  [unu] [GS0ADWURP]

//PTE
//31[*********]11[***]9[*********]0
//  [phys_addr]  [unu] [G0DACWURP]

#include <stdint.h>
#include <kernel/memory/memory.h>
#include <kernel/proc/proc.h>
#include <kernel/global.h>
#include <kernel/misc/panic.h>
extern uint32_t k_end;


uint32_t* table_mappings;


uint8_t paging_flag = 0;

uint32_t pd_entry(uint32_t pte_paddr, uint8_t flags){
	return (pte_paddr | flags);
}

uint8_t flags(uint32_t entry){
	return entry & 0xFF;
}

uint32_t address(uint32_t entry){
	return entry & 0xFFFFFF00;
}

uint32_t v_addr_to_pde(uint32_t addr){
	uint32_t pdindex = (uint32_t)addr >> 22;
    	return pdindex;
}

uint32_t v_addr_to_pte(uint32_t addr){
	uint32_t ptindex = (uint32_t)addr >> 12 & 0x03FF;
    	return ptindex;
}

uint32_t pt_entry(uint32_t p_addr, uint8_t flags){
	return (p_addr | flags);
}

static inline void flush_tlb(unsigned long addr)
{
   asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

void map(uint32_t p_addr,uint32_t v_addr,uint8_t _flags){
	uint32_t pde = v_addr_to_pde(v_addr);
	if(pde == 1023){
		crash_info_t crash;
		crash.description = "Tried to map address to kernel mappings page table";
		crash.extra_info = "We probably just out of memory or somebody tried to map very large address";
		kpanic(crash);
	}
	uint32_t pte = v_addr_to_pte(v_addr);
	uint32_t i_pd_entry = current_page_directory[pde];
	if(!(flags(i_pd_entry) & PAGE_PRESENT)){
		uint32_t pte = kfalloc();
		//printf("New PT at %a\n",pte);
		table_mappings[pde] = pt_entry(pte,_flags);
		if(paging_flag){
			flush_tlb(KERNEL_PT_MAP + pde*4096);
		}
		current_page_directory[pde] = pd_entry(pte,_flags);
	}
	i_pd_entry = current_page_directory[pde];
	uint32_t* k_pt = (uint32_t*)(paging_flag?(KERNEL_PT_MAP + pde*4096):(address(i_pd_entry)));
	uint32_t i_pt_entry = k_pt[pte];
	if(flags(i_pt_entry) & PAGE_PRESENT){
		//kwarn("Trying to remap %p from %p to %p...\n",v_addr,virtual2physical(v_addr),p_addr);
	}
	k_pt[pte] = pt_entry(p_addr, _flags );
	if(paging_flag){
		flush_tlb(v_addr);
	}

}

uint32_t virtual2physical(uint32_t v_addr){
	v_addr = v_addr & 0xFFFFF000;
	uint32_t pde = v_addr_to_pde(v_addr);
	uint32_t pte = v_addr_to_pte(v_addr);
	uint32_t i_pd_entry = current_page_directory[pde];
	if(!(flags(i_pd_entry) & PAGE_PRESENT)){
		//kinfo("PD NOT PRESENT\n");
		return 0;
	}
	uint32_t* k_pt = (uint32_t*)(paging_flag?(KERNEL_PT_MAP + pde*4096):(address(i_pd_entry)));
	uint32_t i_pt_entry = k_pt[pte];
	if(flags(i_pt_entry) & PAGE_PRESENT){
		return address(i_pt_entry);
	}
	
	return 0;
}

uint32_t* copy_page_directory(uint32_t* src){
	uint32_t* new_pdir = kvalloc(4096,4096);
	memcpy(new_pdir,kernel_page_directory,4096);
	if(src == kernel_page_directory){
		return;
	}
	
	
	uint32_t* pt_map = kvalloc(4096,4096);
	new_pdir[1023] = pd_entry(virtual2physical(pt_map),PAGE_PRESENT | PAGE_RW);
	
	set_page_directory(kernel_page_directory,0);
	memcpy(pt_map,0xFFFFF000,4096);
	pt_map[1023] = pt_entry(virtual2physical(pt_map),PAGE_PRESENT | PAGE_RW);
	
	set_page_directory(src,0);
	for(int i=0;i<1023;i++){
		for(int j=0;j<1024;j++){
			uint32_t addr = (i << 22) | (j << 12) | (0);
			if(addr < 0x80000000 || addr > 0xF0000000){
				continue;
			}
				uint32_t real = virtual2physical(addr);
				if(real){
					set_page_directory(new_pdir,0);
					kmpalloc(addr,real,0);
					set_page_directory(src,0);
				}
			
		}
	}
	return new_pdir;
}

//allocates page with given frame or address
void kmpalloc(uint32_t addr, uint32_t frame,uint8_t flags){
	map(frame?frame:addr,addr,flags?flags:(PAGE_PRESENT | PAGE_RW | PAGE_USER)); 
}

//Allocates next page with addr vaddr
uint32_t* knpalloc(uint32_t vaddr){
	uint32_t frame = kfalloc();
	//kinfo("KNPALLOC(%a) - %a\n",vaddr,frame);
	kmpalloc(vaddr,frame,0);
	return(uint32_t*)vaddr;
}

void kralloc(uint32_t region_start,uint32_t region_end){
	for(uint32_t i=region_start;i<region_end;i+=4096){
		//kinfo("Allocating block %a-%a...\n",i,i+4096);
		knpalloc(i);
	}
}

//frees page and frame, also clears pt if needed
void kpfree(uint32_t v_addr){
	uint32_t pde = v_addr_to_pde(v_addr);
	uint32_t pte = v_addr_to_pte(v_addr);
	uint32_t i_pd_entry = current_page_directory[pde];
	if(!(flags(i_pd_entry) & PAGE_PRESENT)){
		return;
	}
	uint32_t* k_pt = (uint32_t*)(paging_flag?(KERNEL_PT_MAP + pde*4096):(address(i_pd_entry)));
	uint32_t i_pt_entry = k_pt[pte];
	if(flags(i_pt_entry) & PAGE_PRESENT){
		uint32_t addr = address(k_pt[pte]);
		k_pt[pte] = 0x0;
		if(pde != 1023){
			uint8_t flag = 1;
			for(int i=0;i<1024;i++){
				if(k_pt[i]){
					flag = 0;
				}
			}
			if(flag){
				current_page_directory[pde] = 0x0;
				kpfree(KERNEL_PT_MAP + pde*4096);
			}
		}
		kffree(addr);
	}
	if(paging_flag){
		flush_tlb(v_addr);
	}
}
//Free only mapping, not frame
void kpfree_virtual(uint32_t v_addr){
	uint32_t pde = v_addr_to_pde(v_addr);
	uint32_t pte = v_addr_to_pte(v_addr);
	uint32_t i_pd_entry = current_page_directory[pde];
	if(!(flags(i_pd_entry) & PAGE_PRESENT)){
		return;
	}
	uint32_t* k_pt = (uint32_t*)(paging_flag?(KERNEL_PT_MAP + pde*4096):(address(i_pd_entry)));
	uint32_t i_pt_entry = k_pt[pte];
	if(flags(i_pt_entry) & PAGE_PRESENT){
		uint32_t addr = address(k_pt[pte]);
		k_pt[pte] = 0x0;
		if(pde != 1023){
			uint8_t flag = 1;
			for(int i=0;i<1024;i++){
				if(k_pt[i]){
					flag = 0;
				}
			}
			if(flag){
				current_page_directory[pde] = 0x0;
				kpfree(KERNEL_PT_MAP + pde*4096);
			}
		}
	}
	if(paging_flag){
		flush_tlb(v_addr);
	}
}

void pagefault_handler(regs_t r){
	uint32_t cr2;
		__asm__ __volatile__ (
        "mov %%cr2, %%eax\n\t"
        "mov %%eax, %0\n\t" : "=m" (cr2)
    : /* no input */
    : "%eax"
    );
	if(get_current_process() && get_current_process()->pid != 1){
		kinfo("Process %s caused page fault at %p\n",get_current_process()->name,cr2);
		proc_exit(get_current_process());
		return;
	}
	mem_check();
	crash_info_t crash;	
	crash.regs = r;	
	crash.description = "Page Fault";
	char message[60];
	memset(message,0,60);
	sprintf(message,"Fault address: %p, error code = 0x%x",cr2,r->err_code);
	crash.extra_info = message;
	kpanic(crash);
	
}

void set_page_directory(uint32_t pdir,uint8_t phys){
	if(!pdir){
		kwarn("Tried to set null page directory!\n");
		return;
	}
	//kinfo("Setting page directory to %a(p=%a)\n",pdir,current_page_directory);
	current_page_directory = (uint32_t*)pdir;
	__asm_set_page_directory(phys?pdir:virtual2physical(pdir));
}

void init_paging(){
	asm("cli");
	kernel_page_directory = (uint32_t*)kfalloc();
	table_mappings = (uint32_t*)kfalloc();
	current_page_directory = kernel_page_directory;
	kernel_page_directory[1023] = pd_entry(table_mappings,PAGE_PRESENT | PAGE_RW);
	table_mappings[1023] = pt_entry(table_mappings,PAGE_PRESENT|PAGE_RW);
	kmpalloc((uint32_t)kernel_page_directory,0,0);
	extern uint32_t k_frame_stack_size;
	for(int i=0;i<0x400000;i+=4096){
		kmpalloc(i,0,0);
	}
	set_page_directory((uint32_t)kernel_page_directory,1);
	enable_paging();
	table_mappings = 0xFFFFF000;
	isr_set_handler(14,pagefault_handler);
	paging_flag = 1;
	kinfo("Paging initialized\n");
	//kmpalloc(0xAA000000,0x1,0);
	//while(1);
	
	
	asm("sti");
	//while(1);
}

uint8_t validate(uint32_t ptr){
	return (ptr != 0) && (virtual2physical(ptr) != 0);
}
