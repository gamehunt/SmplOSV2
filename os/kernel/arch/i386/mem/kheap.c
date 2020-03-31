/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/


#include <kernel/memory/memory.h>
#include <kernel/misc/stat.h>
#include <kernel/global.h>
#include <string.h>
//Simple allocator, alloc: O(n), free: O(n)
static uint32_t allocated;
static uint32_t* heap_start;
static uint32_t* heap_start_static;
//static uint32_t* heap_end;

static mem_t* free = 0;

static uint32_t stat_alloc,stat_free,stat_merges,stat_alloc_total,stat_freed_total,stat_max_load;

void init_kheap(){
	heap_start = kcpalloc(KHEAP_SIZE/4096);
	heap_start_static = heap_start;
	stat_alloc = create_stat("kheap_alloc_times",0);
	stat_free = create_stat("kheap_free_times",0);
	stat_merges = create_stat("kheap_merges",0);
	stat_alloc_total = create_stat("kheap_alloc_total",0);
	stat_freed_total = create_stat("kheap_freed_total",0);
	stat_max_load = create_stat("kheap_max_load",0);
}

static inline mem_t* header(void* alloc){
	return (mem_t*)((uint32_t)alloc - sizeof(mem_t));
} 
static inline void* ptr(mem_t* alloc){
	return (void*)((uint32_t)alloc + sizeof(mem_t));
} 

static mem_t* split(mem_t* orig,uint32_t size){
	if(orig->size <= size+sizeof(mem_t)){
		return 0;
	}
	if(!size){
		return 0;
	}
	uint32_t osize = orig->size;
	
	void* mem = ptr(orig);
	mem_t* newb = (mem_t*)((uint32_t)mem+size);
	newb->size = orig->size - size - sizeof(mem_t);
	orig->size = size;
	//kinfo("SPLIT: %d to %d + %d + %d | REQ: %d\n",osize,orig->size,newb->size,sizeof(mem_t),size);
	return (mem_t*)newb;
}

void free_insert(mem_t* b){
	b->prev = 0;
	b->next = 0;
	if (!validate(free) || (unsigned long)free> (unsigned long)b) {
		if (validate(free)) {
			free->prev = b;
		}
		b->next = free;
		free = b;
		b->prev = 0;
	} else if(validate(free)){
		mem_t *curr = free;
		while (validate(curr->next) && (unsigned long)curr->next < (unsigned long)b) {
			curr = curr->next;
		}
		if(validate(curr->next)){
			b->next = curr->next;
			curr->next->prev = b;
		}
		b->prev = curr;
		curr->next = b;
	}
}

void free_remove(mem_t* b){
	if (!validate(b->prev)) {
		if (validate(b->next)) {
			free = b->next;
			free->prev = 0;
		} else {
			free = 0;
		}
	} else if(validate(b->next)){
		b->prev->next = b->next;
		b->next->prev = b->prev;
	} else{
		b->prev->next = 0;
	}
}

mem_t* free_block(uint32_t size){
	if(!size){
		return 0;
	}
	mem_t* freeb = free;
	if(!validate(freeb)){
		return 0;
	}
	while(validate(freeb)){
		if(freeb->size == size){
			free_remove(freeb);
			return freeb;
		}
		if(freeb->size > size + sizeof(mem_t)){
			//continue;
			mem_t* new_b = split(freeb,size);
			if(validate(new_b)){
				free_insert(new_b);
			}
			//kinfo("After split: %d when req %d\n",freeb->size,size);
			free_remove(freeb);
			return freeb;
		}
		freeb = freeb->next;
		
	}
	return 0;
}

void merge()
{
	mem_t        *curr = free;
	uint32_t	header_curr, header_next;
	while (curr->next) {
		header_curr = (uint32_t)curr;
		header_next = (uint32_t)curr->next;
		if (header_curr + curr->size + sizeof(mem_t) == header_next) {
			i_update_stat(stat_merges,1);
			curr->size += curr->next->size + sizeof(mem_t);
			curr->next = curr->next->next;
			if (curr->next) {
				curr->next->prev = curr;
			} else {
				break;
			}
		}
		curr = curr->next;
	}
	header_curr = (uint32_t)curr;
}
//just allocates memory
uint32_t* kmalloc(uint32_t size){
	if(!size || size >= KHEAP_SIZE){
		crash_info_t crash;
		crash.description = "KHEAP: Invalid allocation";
		char message[128]; 
		sprintf(message,"Tried to allocate block of size %d\n",size);
		crash.extra_info = message;
		crash.regs = 0;
		kpanic(crash);
	}
	i_update_stat(stat_alloc,1);
	mem_t* block = free_block(size);
	if(validate(block)){
		//kinfo("FREE\n");
		i_update_stat(stat_alloc_total,size);
		i_update_stat(stat_max_load,size);
		block->next = 0xAABBCCDD;
		block->prev = 0xAABBCCDD;
		return ptr(block);
	}else{
		//
		if((uint32_t)heap_start + sizeof(mem_t)+size >= (uint32_t)heap_start_static+KHEAP_SIZE){
			crash_info_t crash;
			crash.description = "KHEAP: Out of memory";
			char message[128]; 
			sprintf(message,"Tried to allocate block of size %d when %d already allocated\n",size,(uint32_t)heap_start-(uint32_t)heap_start_static);
			crash.extra_info = message;
			crash.regs = 0;
			kpanic(crash);
		}
		i_update_stat(stat_alloc_total,size);
		i_update_stat(stat_max_load,size);
		mem_t* nblock = heap_start;
		heap_start = (uint32_t*)((uint32_t)heap_start + sizeof(mem_t)+size);
		nblock->size = size;
		nblock->prev = 0xAABBCCDD;
		nblock->next = 0xAABBCCDD;
		
		return ptr(nblock);
	}
	return 0;
}

//frees memory. 
void kfree(uint32_t* addr){
	
	//return; //TODO find another bug in kfree
	
	mem_t* block = header(addr);
	
	if(block->size >= KHEAP_SIZE || block->next != 0xAABBCCDD || block->prev != 0xAABBCCDD){
		return;
		
	}
	
	memset(addr,0,block->size);
	
	i_update_stat(stat_free,1);
	i_update_stat(stat_freed_total,block->size);
	i_update_stat(stat_max_load,-block->size);
	free_insert(block);
	//merge();
}

//allocates aligned memory, should be freed as ((void**) ptr)[-1]
uint32_t* kvalloc(uint32_t size,uint32_t alignment){
	size_t request_size = size + alignment;
    char* buf =  kmalloc(request_size);

    size_t remainder = ((size_t)buf) % alignment;
    size_t offset = alignment - remainder;
    char* ret = buf + (unsigned char)offset;

    // store how many extra bytes we allocated in the byte just before the
    // pointer we return
    *(unsigned char*)(ret - 1) = offset;
	return (uint32_t*)ret;
}

//frees aligned ptr
void kvfree(uint32_t* aligned_ptr) {
    int offset = *(((char*)aligned_ptr) - 1);
    kfree(((char*)aligned_ptr) - offset);
}

//reallocates memory, currently just do new allocation and copy contents of old pointer to it !!!
uint32_t* krealloc(uint32_t* ptr,uint32_t newsize){
	uint32_t* new_alloc = kmalloc(newsize);
	memmove(new_alloc,ptr,header(ptr)->size >= newsize?newsize:header(ptr)->size);
	kfree(ptr);
	return new_alloc;
}
