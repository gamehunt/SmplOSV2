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

static uint32_t k_frame_stack_esp = 0;

static uint32_t k_frame_stack_size = 0;

static uint16_t stat,stat1,stat2;

uint32_t k_frame_stack_pop(){
	if(k_frame_stack_esp >= k_frame_stack_size){	
		crash_info_t crash;
		crash.description = "PMM: Out of memory";	
		char buffer[64];
		sprintf(buffer,"K_FRAME_STACK_ESP = %d | K_FRAME_STACK_SIZE = %d\n",k_frame_stack_esp,k_frame_stack_size);	
		crash.extra_info = buffer;
		kpanic(crash);
		return 0;
	} 
	uint32_t addr = k_frame_stack[k_frame_stack_esp];
	//kinfo("Popped from frame stack: %a(esp=%d)\n",addr,k_frame_stack_esp);
	k_frame_stack_esp++;
	return addr;
}

void k_frame_stack_push(uint32_t frame){
	if(!k_frame_stack_size){
			kwarn("Tried to push frame in a full stack\n");
			return;
	}
	k_frame_stack_esp--;
	k_frame_stack[k_frame_stack_esp] = frame;
	
}


void init_pmm(multiboot_info_t *mbt){
	kinfo("Kernel frame stack at %a\n",k_frame_stack);
	stat = create_stat("pmm_stack_pushed",0);
	stat1 = create_stat("pmm_stack_popped",0);
	stat2 = create_stat("pmm_peak",0);
	multiboot_memory_map_t* mmap = mbt->mmap_addr;
	while((uint32_t)mmap < mbt->mmap_addr+ mbt->mmap_length) {
		kinfo("0x%e - 0x%e - 0x%x\n",mmap->addr,mmap->addr+mmap->len,mmap->type);
		if(mmap->type == 1){
			for(int i = 0; i< mmap->len; i+= 4096){
				if(mmap->addr+i > 0x300000){
					memset(&k_frame_stack[k_frame_stack_size],0,4);
					k_frame_stack[k_frame_stack_size] = mmap->addr+i;
					k_frame_stack_size++;
					//kinfo("Added to frame stack: %a(esp=%d)\n",k_frame_stack[k_frame_stack_size-1],k_frame_stack_size-1);
				}
			}
		}
		mmap = (multiboot_memory_map_t*) ( (unsigned int)mmap + mmap->size + sizeof(mmap->size) );
	}
	kinfo("Frame stack size: %d\n",k_frame_stack_size);
}

//Allocates frame

uint32_t kfalloc(){

	i_update_stat(stat1,1);
	i_update_stat(stat2,1);
	uint32_t frame =  k_frame_stack_pop();
	return frame;
}

//Frees frame
void kffree(uint32_t addr){
	i_update_stat(stat,1);
	i_update_stat(stat2,-1);
	k_frame_stack_push(addr);
}




