#include <kernel/memory/memory.h>
#include <kernel/global.h>
#include <string.h>
//Very,very,VERY simple allocator, alloc: O(n), free: O(n)
uint32_t allocated;
uint32_t* heap_start;

uint32_t* zones[KHEAP_SIZE/4];
uint32_t z_esp = 0;

void init_kheap(){
	heap_start = kcpalloc(4);
	memset(zones,0,KHEAP_SIZE/4);
}

//just allocates memory
uint32_t* kmalloc(uint32_t size){
	//kinfo("KMALLOC(%d)\n",size);
	if(size < 4){
//		kwarn("Tried to allocate less than 4 bytes(%d): increasing allocation size...\n",size);
		size = 4;
	}
	if(KHEAP_SIZE <= size + sizeof(uint32_t) + allocated){
		kerr("Failed to alloc %d: OUT OF MEM\n",size);
		return 0;
	}
	if(z_esp){
		for(uint32_t i=0;i<KHEAP_SIZE/4;i++){
			if(!zones[i]){
				continue;
			}
			if(zones[i][1] > size){
				uint32_t old = zones[i][1];
				uint32_t* res = zones[i];
				res[0] = size;
				allocated += (size+sizeof(uint32_t));
				zones[i]=(uint32_t)zones[i] + (size+sizeof(uint32_t));
				zones[i][1] = old - size;
				//kinfo("KMALLOC-END(%d)\n",size);
				return res+1;
			}else if(zones[i][1] == size){
				uint32_t* addr = zones[i];
				zones[i] = 0;
				addr[0] = size;
				allocated += (size+sizeof(uint32_t));
				free_esp();
				//kinfo("KMALLOC-END(%d)\n",size);
				return addr+1;
			}
		}
	}
	uint32_t* heap_ptr = (uint32_t)heap_start + allocated;
	*heap_ptr = size; 
	allocated+=(size+sizeof(uint32_t));
	//kinfo("KMALLOC-END(%d)\n",size);
	return (heap_ptr+1);
}

void free_esp(){
	if(z_esp+1 > KHEAP_SIZE/4 || zones[z_esp+1]){
		for(uint32_t i=0;i<KHEAP_SIZE/4;i++){
			if(!zones[i]){
				z_esp = i;
				break;
			}
		}
	}else{
		z_esp+=1;
	}
}

//frees memory. 
void kfree(uint32_t* addr){
	if(addr[-1] != 0){
		uint32_t size = addr[-1];
		allocated -= (addr[-1] + sizeof(uint32_t));
		addr[-1] = 0;
		addr[0] = size;
		zones[z_esp] = addr-1;
		free_esp();
	}else{
		kwarn("We probably tried to free unallocated memory\n");
	}
}

//allocates aligned memory !! Wastes lot's of memory if alignment is large
uint32_t* kvalloc(uint32_t size,uint32_t alignment){
	kwarn("kvalloc stub\n");
	return kmalloc(size);
}

//reallocates memory, currently just do new allocation and copy contents of old pointer to it !!!
uint32_t* krealloc(uint32_t* ptr,uint32_t newsize){
	uint32_t* addr = kmalloc(newsize);
	memmove(addr,ptr,ptr[-1]);
	kfree(ptr);
	return addr;
}
