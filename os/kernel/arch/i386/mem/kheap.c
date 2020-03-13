/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/


#include <kernel/memory/memory.h>
#include <kernel/misc/stat.h>
#include <kernel/global.h>
#include <string.h>
//Very,very,VERY simple allocator, alloc: O(n), free: O(n)
static uint32_t allocated;
static uint32_t* heap_start;
static uint32_t* heap_start_static;
//static uint32_t* heap_end;

struct mem_block{
	uint32_t size;
	struct mem_block* prev;
	struct mem_block* next;
};
typedef struct mem_block mem_t;

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
	if(orig->size < size+sizeof(mem_t)){
		return 0;
	}
	void* mem = ptr(orig);
	mem_t* newb = (mem_t*)((uint32_t)mem+size);
	newb->size = orig->size - size - sizeof(mem_t);
	orig->size -= size+sizeof(mem_t);
	return (mem_t*)newb;
}

void free_insert(mem_t* b){
	b->prev = 0;
	b->next = 0;
	if (!free || (unsigned long)free> (unsigned long)b) {
		if (free) {
			free->prev = b;
		}
		b->next = free;
		free = b;
	} else {
		mem_t *curr = free;
		while (curr->next && (unsigned long)curr->next < (unsigned long)b) {
			curr = curr->next;
		}
		b->next = curr->next;
		curr->next = b;
	}
}

void free_remove(mem_t* b){
	if (!b->prev) {
		if (b->next) {
			free = b->next;
		} else {
			free = 0;
		}
	} else {
		b->prev->next = b->next;
	}
	if (b->next) {
		b->next->prev = b->prev;
	}
}

mem_t* free_block(uint32_t size){
	mem_t* freeb = free;
	if(!freeb){
		return 0;
	}
	while(freeb){
		if(freeb->size == size){
			free_remove(freeb);
			return freeb;
		}
		if(freeb->size > size){
			mem_t* new_b = split(freeb,size);
			free_insert(new_b);
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
	i_update_stat(stat_alloc,1);
	mem_t* block = free_block(size);
	if(block){
		//kinfo("FREE\n");
		i_update_stat(stat_alloc_total,size);
		i_update_stat(stat_max_load,size);
		block->next = 0;
		block->prev = 0;
		return ptr(block);
	}else{
		if((uint32_t)heap_start + sizeof(mem_t)+size > heap_start_static+KHEAP_SIZE*8){
			kerr("Out of memory\n");
			return 0;
		}
		i_update_stat(stat_alloc_total,size);
		i_update_stat(stat_max_load,size);
		mem_t* nblock = heap_start;
		heap_start = (uint32_t*)((uint32_t)heap_start + sizeof(mem_t)+size);
		nblock->size = size;
		nblock->prev = 0;
		nblock->next = 0;
		return ptr(nblock);
	}
	return 0;
}


//frees memory. 
void kfree(uint32_t* addr){
	//kinfo("FREE %a\n",addr);
	i_update_stat(stat_free,1);
	mem_t* block = header(addr);
	//kinfo("FREE BLOCK INFO : %d %a %a\n",block->size,block->next,block->prev);
	i_update_stat(stat_freed_total,block->size);
	i_update_stat(stat_max_load,-block->size);
	free_insert(block);
	merge();
}

//allocates aligned memory !! Wastes lot's of memory if alignment is large
uint32_t* kvalloc(uint32_t size,uint32_t alignment){
	return kmalloc(size);
}

//reallocates memory, currently just do new allocation and copy contents of old pointer to it !!!
uint32_t* krealloc(uint32_t* ptr,uint32_t newsize){
	uint32_t* new_alloc = kmalloc(newsize);
	memmove(new_alloc,ptr,header(ptr)->size >= newsize?newsize:header(ptr)->size);
	kfree(ptr);
	return new_alloc;
}
