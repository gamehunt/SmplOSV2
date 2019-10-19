//PDE
//31[********]11[***]9[*********]0
//  [pte_addr]  [unu] [GS0ADWURP]

//PTE
//31[*********]11[***]9[*********]0
//  [phys_addr]  [unu] [G0DACWURP]

#include <stdint.h>
#include <kernel/memory/memory.h>
#include <kernel/global.h>
extern uint32_t k_end;


uint32_t table_mappings[1024] __attribute__((aligned(4096))); // mappings for pte's is last pte

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

void map(uint32_t p_addr,uint32_t v_addr){
	uint32_t pde = v_addr_to_pde(v_addr);
	if(pde == 1023){
		crash_info_t crash;
		crash.description = "kpalloc() tried to map address to kernel mappings page table";
		crash.extra_info = "We probably just out of memory or somebody tried to map very large address";
		kpanic(crash);
	}
	uint32_t pte = v_addr_to_pte(v_addr);
	uint32_t i_pd_entry = kernel_page_directory[pde];
	if(!(flags(i_pd_entry) & PAGE_PRESENT)){
		uint32_t pte = kfalloc();
		//printf("New PT at %a\n",pte);
		table_mappings[pde] = pt_entry(pte,PAGE_PRESENT|PAGE_RW );
		if(paging_flag){
			flush_tlb(0xFFC00000 + pde*4096);
		}
		kernel_page_directory[pde] = pd_entry(pte,PAGE_PRESENT | PAGE_RW );
	}
	i_pd_entry = kernel_page_directory[pde];
	uint32_t* k_pt = (uint32_t*)(paging_flag?(0xFFC00000 + pde*4096):(address(i_pd_entry)));
	uint32_t i_pt_entry = k_pt[pte];
	if(flags(i_pt_entry) & PAGE_PRESENT){
		kwarn("Trying to remap %a...\n",v_addr);
	}
	k_pt[pte] = pt_entry(p_addr, PAGE_PRESENT | PAGE_RW );
	if(paging_flag){
		flush_tlb(v_addr);
	}

}

uint32_t virtual2physical(uint32_t v_addr){
	uint32_t pde = v_addr_to_pde(v_addr);
	uint32_t pte = v_addr_to_pte(v_addr);
	uint32_t i_pd_entry = kernel_page_directory[pde];
	if(!(flags(i_pd_entry) & PAGE_PRESENT)){
		return 0;
	}
	uint32_t* k_pt = (uint32_t*)(paging_flag?(0xFFC00000 + pde*4096):(address(i_pd_entry)));
	uint32_t i_pt_entry = k_pt[pte];
	if(flags(i_pt_entry) & PAGE_PRESENT){
		return address(i_pt_entry);
	}
	return 0;
}

uint32_t* copy_page_directory(uint32_t* src){
	uint32_t* new_pdir = (uint32_t*)kpalloc();
	memcpy(new_pdir,src,4096);
	return paging_flag?virtual2physical(new_pdir):new_pdir;
	//return new_pdir;
}

//allocates page with given frame or address
void kmpalloc(uint32_t addr, uint32_t frame){
	map(frame?frame:addr,addr); 
}

//Just allocates next page with frame given from kfalloc()

uint32_t* kpalloc(){
	uint32_t addr = kfalloc();
	kmpalloc(addr,0);
	return (uint32_t*) addr;
}

//Allocates next page with addr vaddr
uint32_t* knpalloc(uint32_t vaddr){
	kmpalloc(vaddr,kfalloc());
}

//Allocates n pages, which mapped as continious
uint32_t* kcpalloc(uint32_t n){
	uint32_t* ptr = kpalloc();
		
	for(int i=0;i<n-1;i++){
		knpalloc((uint32_t)ptr + 4096);
	}
	return ptr;
}

//frees page and frame, also clears pt if needed
void kpfree(uint32_t v_addr){
	uint32_t pde = v_addr_to_pde(v_addr);
	uint32_t pte = v_addr_to_pte(v_addr);
	uint32_t i_pd_entry = kernel_page_directory[pde];
	if(!(flags(i_pd_entry) & PAGE_PRESENT)){
		return;
	}
	uint32_t* k_pt = (uint32_t*)(paging_flag?(0xFFC00000 + pde*4096):(address(i_pd_entry)));
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
				kernel_page_directory[pde] = 0x0;
				kpfree(0xFFC00000 + pde*4096);
			}
		}
		kffree(addr);
	}
	if(paging_flag){
		flush_tlb(v_addr);
	}
	
}
void init_paging(){
	asm("cli");
	kernel_page_directory = (uint32_t*)kfalloc();
	kernel_page_directory[1023] = pd_entry(&table_mappings,PAGE_PRESENT | PAGE_RW);
	kmpalloc((uint32_t)kernel_page_directory,0);
	extern uint32_t k_frame_stack_size;
	for(int i=0;i<0x2000000;i+=4096){
		kmpalloc(i,0);
	}
	set_page_directory((uint32_t)kernel_page_directory);
	enable_paging();
	paging_flag = 1;
	kinfo("Paging initialized\n");
	asm("sti");
	//while(1);
}



