/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/memory/memory.h>
#include <kernel/global.h>
#include <kernel/misc/panic.h>
#include <kernel/misc/stat.h>
extern uint32_t k_end;

uint32_t* k_frame_stack = &k_end;
static uint32_t* k_temp_frame_stack;

static long k_frame_stack_esp = 0;
static long k_temp_frame_stack_esp = 0;

static uint32_t k_frame_stack_size;

static uint16_t stat,stat1;

uint32_t k_frame_stack_pop(){
	if(k_frame_stack_esp < 0){	
		crash_info_t crash;
		crash.description = "Out of memory";		
		kpanic(crash);	
		return 0;
	} 
	uint32_t addr = k_frame_stack[k_frame_stack_esp-1];
	k_frame_stack_esp--;
	return addr;
}

void k_frame_stack_push(uint32_t frame){
	k_frame_stack[k_frame_stack_esp] = frame;
	k_frame_stack_esp++;
}

void k_temp_frame_stack_push(uint32_t frame){
	k_temp_frame_stack[k_temp_frame_stack_esp] = frame;
	k_temp_frame_stack_esp++;
}

uint32_t k_temp_frame_stack_pop(){
	if(k_temp_frame_stack_esp < 0){		
		crash_info_t crash;
		crash.description = "Out of memory";		
		kpanic(crash);
		return 0;
	} 
	uint32_t addr = k_temp_frame_stack[k_temp_frame_stack_esp-1];
	k_temp_frame_stack_esp--;
	return addr;
}

void init_pmm(multiboot_info_t *mbt){
	kinfo("Kernel frame stack at %a\n",k_frame_stack);
	stat = create_stat("pmm_stack_pushed",0);
	stat1 = create_stat("pmm_stack_popped",0);
	multiboot_memory_map_t* mmap = mbt->mmap_addr;
	uint32_t k_frame_stack_size = 0;
	while((uint32_t)mmap < mbt->mmap_addr+ mbt->mmap_length) {
		kinfo("0x%e - 0x%e - 0x%x\n",mmap->addr,mmap->addr+mmap->len,mmap->type);
		if(mmap->type == 1){
			for(int i = 0; i< mmap->len; i+= 4096){
				k_frame_stack_size++;
			}
		}
		mmap = (multiboot_memory_map_t*) ( (unsigned int)mmap + mmap->size + sizeof(mmap->size) );
	}
	
	memset(k_frame_stack,0,k_frame_stack_size);
	k_temp_frame_stack = (uint32_t*)((uint32_t)&k_frame_stack + k_frame_stack_size*sizeof(uint32_t));
	memset(k_temp_frame_stack,0,k_frame_stack_size);
	mmap = mbt->mmap_addr;
	uint32_t useful_mem = 0;
	while((uint32_t)mmap < mbt->mmap_addr + mbt->mmap_length) {
		if(mmap->type == 1){
			for(int i = 0; i< mmap->len; i+= 4096){
				if(mmap->addr+i > 0x1000000){
					k_temp_frame_stack_push(mmap->addr+i);
					useful_mem++;
				}
			}
		}
		mmap = (multiboot_memory_map_t*) ( (unsigned int)mmap + mmap->size + sizeof(mmap->size) );
	}
	//reverse the stack to get smallest frame first (needs for correct pte allocation)
	for(int i=0;i<useful_mem;i++){
		uint32_t v = k_temp_frame_stack_pop();
		k_frame_stack_push(v);
	}
	kinfo("Initialized frame stack with size %l, esp = %l\n",k_frame_stack_size,k_frame_stack_esp);
}

//Allocates frame

uint32_t kfalloc(){

	i_update_stat(stat1,1);
	uint32_t frame =  k_frame_stack_pop();
	return frame;
}

//Frees frame
void kffree(uint32_t addr){
	i_update_stat(stat,1);
	k_frame_stack_push(addr);
}




