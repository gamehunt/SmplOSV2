/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdlib.h>
#include <kernel/memory/memory.h>
#include <sys/syscall.h>

#ifndef __smplos_libk



static uint32_t* heap_start = USER_HEAP;
static uint32_t heap_size = USER_HEAP_SIZE;

#define VALIDATE_PTR(ptr) (USER_HEAP<=ptr && ptr<= heap_size)

static mem_t* free_list = 0;

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
	if(!size){
		return 0;
	}
	uint32_t osize = orig->size;
	
	void* mem = ptr(orig);
	mem_t* newb = (mem_t*)((uint32_t)mem+size);
	newb->size = orig->size - size - sizeof(mem_t);
	orig->size = size;
	//kinfo("SPLIT: %d to %d + %d\n",osize,orig->size,newb->size);
	return (mem_t*)newb;
}

void free_insert(mem_t* b){
	b->prev = 0;
	b->next = 0;
	if (!VALIDATE_PTR(free_list) || (unsigned long)free_list> (unsigned long)b) {
		if (free_list) {
			free_list->prev = b;
		}
		b->next = free_list;
		free_list = b;
	} else if(VALIDATE_PTR(free_list)){
		mem_t *curr = free;
		while (VALIDATE_PTR(curr->next) && (unsigned long)curr->next < (unsigned long)b) {
			curr = curr->next;
		}
		if(VALIDATE_PTR(curr->next)){
			b->next = curr->next;
			curr->next->prev = b;
		}
		b->prev = curr;
		curr->next = b;
	}
}

void free_remove(mem_t* b){
	if (!VALIDATE_PTR(b->prev)) {
		if (VALIDATE_PTR(b->next)) {
			free_list = b->next;
		} else {
			free_list = 0;
		}
	} else if(VALIDATE_PTR(b->next)){
		b->prev->next = b->next;
		b->next->prev = b->prev;
	} else{
		b->prev->next = 0;
	}
}

mem_t* free_block(uint32_t size){
	//return 0;
	if(!size){
		return 0;
	}
	mem_t* freeb = free_list;
	if(!VALIDATE_PTR(freeb)){
		return 0;
	}
	while(VALIDATE_PTR(freeb)){
		if(freeb->size == size){
			free_remove(freeb);
			return freeb;
		}
		
		if(freeb->size > size){
			mem_t* new_b = split(freeb,size);
			if(VALIDATE_PTR(new_b)){
				free_insert(new_b);
			}
			free_remove(freeb);
			return freeb;
		}
		
		freeb = freeb->next;
	}
	return 0;
}

void fix_user_heap(){
	mem_t* block = heap_start;
	while(block->size){
		block = (mem_t*)((uint32_t)block + sizeof(mem_t) + block->size);
		heap_start = block;
	}
}

#endif

void* malloc(size_t size){
	#ifdef __smplos_libk 
		return (void*)kmalloc(size);
	#else
		if(!size){
			return 0;
		}
		while(size >= heap_size){
			heap_size += 4096;
			sys_sbrk(4096);
		}
		mem_t* block = free_block(size);
		if(block){
			block->next = 0xAABBCCDD;
			block->prev = 0xAABBCCDD;
			return ptr(block);
		}else{
			if((uint32_t)heap_start + sizeof(mem_t) + size >= (uint32_t)USER_HEAP+heap_size){
				heap_size += 4096;
				sys_sbrk(4096);
			}
			mem_t* nblock = heap_start;
			heap_start = (uint32_t*)((uint32_t)heap_start + sizeof(mem_t)+size);
			nblock->size = size;
			nblock->prev = 0xAABBCCDD;
			nblock->next = 0xAABBCCDD;
			return ptr(nblock);
	}
	#endif
	return 0;
}

void* valloc(size_t size,uint32_t alig){
	#ifdef __smplos_libk
		return (void*)kvalloc(size,alig);
	#else
		return malloc(size);
	#endif
	return 0;
}
void free(void* mem){
	#ifdef __smplos_libk
		return kfree(mem);
	#else
		mem_t* block = header(mem);
		
		if(block->size >= heap_size || block->next != 0xAABBCCDD || block->prev != 0xAABBCCDD){
			return;
		
		}
		
		free_insert(block);
	#endif
}
void* realloc(uint32_t* ptr,size_t size){
	#ifdef __smplos_libk
		return (void*)krealloc(ptr,size);
	#else
		uint32_t* new_alloc = malloc(size);
		memmove(new_alloc,ptr,header(ptr)->size >= size?size:header(ptr)->size);
		free(ptr);
		return new_alloc;
	#endif
	return 0;
}



