#include <kernel/memory/memory.h>
#include <kernel/global.h>
#include <kernel/misc/panic.h>
extern uint32_t k_end;

uint32_t* k_frame_stack = &k_end;
uint32_t* k_temp_frame_stack;

long k_frame_stack_esp = 0;
long k_temp_frame_stack_esp = 0;

uint32_t k_frame_stack_size;

uint32_t k_frame_stack_pop(){
	if(k_frame_stack_esp < 0){	
		crash_info_t crash;
		crash.description = "Out of memory";		
		kpanic(crash);	
		return 0;
	} 
	//while(1);
	uint32_t addr = k_frame_stack[k_frame_stack_esp-1];
	k_frame_stack_esp--;
	//while(1);
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
	multiboot_memory_map_t* mmap = mbt->mmap_addr;
	uint32_t k_frame_stack_size = 0;
	//kinfo("%a\n",(uint32_t)(mbt->mmap_addr));
	//while(1);	
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
	k_temp_frame_stack = (uint32_t*)((uint32_t)&k_end + k_frame_stack_size*sizeof(uint32_t));
	memset(k_temp_frame_stack,0,k_frame_stack_size);
	mmap = mbt->mmap_addr;
	uint32_t useful_mem = 0;
	while((uint32_t)mmap < mbt->mmap_addr + mbt->mmap_length) {
		if(mmap->type == 1){
			for(int i = 0; i< mmap->len; i+= 4096){
				//kinfo("0x%e 0%x\n",mmap->addr+i,((uint32_t)&k_end+k_frame_stack_size));
				if(mmap->addr+i > 0x1000000){
		//			kinfo("%a\n",mmap->addr+i);
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
		//kinfo("%a; esp = %d\n",v,k_temp_frame_stack_esp);
		k_frame_stack_push(v);
	}
	kinfo("Initialized frame stack with size %l, esp = %l\n",k_frame_stack_size,k_frame_stack_esp);
}

//Allocates frame

uint32_t kfalloc(){
	uint32_t frame =  k_frame_stack_pop();
	//while(1);
	return frame;
}

//Frees frame
void kffree(uint32_t addr){
	k_frame_stack_push(addr);
}




