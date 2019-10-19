#include <kernel/memory/memory.h>
#include <kernel/global.h>
//Very,very,VERY simple allocator, alloc: O(n), free: O(n)

typedef struct{
	uint32_t size;
	uint32_t addr;
	uint8_t free;
} mem_t;

mem_t memory[KHEAP_MAX_SIZE];

uint32_t mid;
uint32_t allocated;
uint32_t* heap_ptr;
uint32_t cur_size;

void init_kheap(){
	//Pre alloc all kernel heap
	//while(1);
	heap_ptr = kcpalloc(KHEAP_SIZE/4096);	
	
	memset(memory,0,KHEAP_MAX_SIZE*sizeof(mem_t));
	kinfo("Kernel heap located at %a (%d KB)\n",(uint32_t)heap_ptr,KHEAP_SIZE/1024);
	mid = 0;
	allocated = 0;
	cur_size = KHEAP_SIZE;
}

void optimize_list(){
	//Sort array by addresses
	int i, j;
	mem_t temp;
	if(!mid){
		return;
	}
 	for (i = 0; i < (mid - 1); ++i)
 	{
      		for (j = 0; j < mid - 1 - i; ++j )
      		{
           		if (memory[j].addr > memory[j+1].addr)
           		{
                		temp = memory[j+1];
                		memory[j+1] = memory[j];
                		memory[j] = temp;
           		}
      		}
 	}
//	print_memory();
//	printf("Sorted\n");
	for(int i=0;i<mid;i++){
		if(memory[i].free){
			uint32_t second = i+1;
			while(second < mid && memory[second].free){
				memory[i].size+=memory[second].size;
				memory[second].size = KHEAP_MAX_SIZE;
				second++;
			}
			i = second;
		}
	}
//	printf("Merged\n");
	mem_t new_memory[KHEAP_MAX_SIZE];
	uint32_t new_memory_id = 0;
	for(int i=0;i<mid;i++){
		if(memory[i].size < KHEAP_MAX_SIZE){
			new_memory[new_memory_id] = memory[i];
			new_memory_id++;
		}
	}
	mid = new_memory_id;
	for(int i=0;i<mid;i++){
		memory[i] = new_memory[i];
	}
	//printf("Copied\n");
	
}




//just allocates memory
uint32_t* kmalloc(uint32_t size){
	//printf("%d\n",allocated);
	//print_memory();
	while(cur_size-allocated < size){
		cur_size+=4096;
		knpalloc((uint32_t)heap_ptr + cur_size + 1);
	}
	for(int i=0;i<mid;i++){
		if(memory[i].free && memory[i].size>=size){
			if(memory[i].size > size){
				memory[i].size -= size;
				uint32_t addr = memory[i].addr;
				memory[i].addr += size;
				mem_t newb;
				newb.addr = addr;	
				newb.free = 0;
				newb.size = size;
	
				memory[mid] = newb;
				mid++;
				allocated+=size;
				if(mid > KHEAP_MAX_SIZE){
					crash_info_t crash;
					crash.description = "Global allocator fault";
					crash.extra_info = "Block list overflow";
					kpanic(crash);
				}
				optimize_list();
				return addr;
			}else{
				memory[i].free = 0;
				allocated+=size;
				optimize_list();
				return memory[i].addr;
			}
			
		}
	}
	uint32_t addr = (uint32_t)heap_ptr+allocated;
	mem_t alloc;
	alloc.size = size;
	alloc.addr = addr;
	alloc.free = 0;

	allocated+=size;
	memory[mid] = alloc;
	mid++;
	if(mid > KHEAP_MAX_SIZE){
		crash_info_t crash;
		crash.description = "Global allocator fault";
		crash.extra_info = "Block list overflow";
		kpanic(crash);
	}
	optimize_list();
	return addr;
}

//frees memory. DONT WORK FOR KVALLOC()!!!
void kfree(uint32_t addr){
	for(int i=0;i<mid;i++){
		if(memory[i].addr == addr && !memory[i].free){
			memory[i].free = 1;
			allocated -= memory[i].size;
			break;
		}
	}
	optimize_list();
}

void print_memory(){
	kinfo("Memory list contains %d cache entries\n",mid);
	for(int i=0;i<mid;i++){
		printf("%a - %d - %s\n",memory[i].addr,memory[i].size,memory[i].free?"free":"used");
	}
	kinfo("Current heap size: %d, allocated %d\n",cur_size, allocated);
}

//allocates aligned memory !! Wastes lot's of memory if alignment is large
uint32_t* kvalloc(uint32_t size,uint32_t alignment){
	uint32_t* mem = kmalloc(size+sizeof(uint32_t*)+alignment-1);
	uint32_t* ptr = (uint32_t**)(((uint32_t)mem + (alignment - 1) + sizeof(uint32_t*)) & ~(alignment - 1));
	((uint32_t**)ptr)[-1] = mem;
	return ptr;
}

//free's aligned memory. Use this for aligned allocations 
void kvfree(uint32_t* addr){
	kfree(((uint32_t**)addr)[-1]);
}

//reallocates memory, currently just do new allocation and copy contents of old pointer to it !!! SLOW AND WASTEFUL TODO: optimize that
uint32_t* krealloc(uint32_t* ptr,uint32_t newsize){
	uint32_t* new_pointer = kmalloc(newsize);
	for(int i=0;i<mid;i++){
		//printf("%a %a\n",memory[i].addr,(uint32_t)ptr);
		if(memory[i].addr == (uint32_t)ptr && !memory[i].free){
			//printf("Moving %a to %a (%d bytes)\n",ptr,new_pointer,newsize>memory[i].size?memory[i].size:newsize);
			memmove(new_pointer,ptr,newsize>memory[i].size?memory[i].size:newsize);
			break;
		}
	}
	kfree(ptr);
	return new_pointer;
}
