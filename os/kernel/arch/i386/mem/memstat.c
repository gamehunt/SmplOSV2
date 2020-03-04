#include <kernel/memory/memory.h>

void mem_stat(){
	kinfo("PMM STAT:\n");
	kinfo("ALLOC: %d FREED: %d PEAK: %d\n",get_stat("pmm_stack_popped"),get_stat("pmm_stack_pushed"),get_stat("pmm_peak"));
	kinfo("VMM STAT:\n");
	kinfo("TA: %d KB(%d times) TF: %d KB (%d times) - %d merged - %d KB peak\n",get_stat("kheap_alloc_total")/1024,get_stat("kheap_alloc_times"),get_stat("kheap_freed_total")/1024,get_stat("kheap_free_times"),get_stat("kheap_merges"),get_stat("kheap_max_load")/1024);
}
