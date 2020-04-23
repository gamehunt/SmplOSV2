/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdlib.h>
#include <kernel/memory/memory.h>
#include <kernel/proc/sync.h>
#include <sys/syscall.h>

#ifndef __smplos_libk

static uint32_t* heap_start = USER_HEAP;
static uint32_t heap_size = USER_HEAP_SIZE;


#define VALIDATE_PTR(ptr) (USER_HEAP<=ptr && ptr<= USER_HEAP+heap_size)
#define validate VALIDATE_PTR 

static mem_t* free_list = 0;

static inline mem_t* header(void* alloc){
	return (mem_t*)((uint32_t)alloc - sizeof(mem_t));
} 
static inline void* ptr(mem_t* alloc){
	return (void*)((uint32_t)alloc + sizeof(mem_t));
} 
static mem_t* split(mem_t* orig,uint32_t size){
	//sys_echo("Split:",size);
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
	//sys_echo("Inserting block of size",b->size);
	b->prev = 0;
	b->next = 0;
	if (!validate(free_list) || (unsigned long)free_list> (unsigned long)b) {
		if (validate(free_list)) {
			free_list->prev = b;
			b->next = free_list;
		}
		free_list = b;
	} else if(validate(free_list)){
		mem_t *curr = free_list;
		while (validate(curr->next) && (unsigned long)curr->next < (unsigned long)b) {
			curr = curr->next;
		}
		if(validate(curr->next)){
			b->next = curr->next;
			b->next->prev = b;
		}
		b->prev = curr;
		curr->next = b;
	}
}

void free_remove(mem_t* b){
	if (!validate(b->prev)) {
		if (validate(b->next)) {
			free_list = b->next;
			free_list->prev = 0;
		} else {
			free_list = 0;
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
	mem_t* freeb = free_list;
	if(!validate(freeb)){
		return 0;
	}
	while(validate(freeb)){
		if(freeb->size == size){
			free_remove(freeb);
			return freeb;
		}
		if(freeb->size > size + sizeof(mem_t)){
			mem_t* new_b = split(freeb,size);
			if(validate(new_b)){
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

static uint8_t volatile memory_lock = 0;

static void lock(){
	#ifndef __smplos_libk
	while(__sync_lock_test_and_set(&memory_lock, 0x01)) {
		sys_yield();
	}
	#endif
}

static void unlock(){
	#ifndef __smplos_libk
	__sync_lock_release(&memory_lock);
	#endif
}

void* __attribute__ ((malloc)) __unsafe_malloc(size_t size){
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
			//sys_echo("Using freed\n",0);
			block->next = 0xAABBCCDD;
			block->prev = 0xAABBCCDD;
			return ptr(block);
		}else{
			while((uint32_t)heap_start + sizeof(mem_t) + size >= (uint32_t)USER_HEAP+heap_size){
				heap_size += 4096;
				sys_sbrk(4096);
				//sys_echo("SBRK\n",0);
			}
			//sys_echo("Making new block\n",0);
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
void* __attribute__ ((malloc)) __unsafe_realloc(uint32_t* ptr,size_t size){
	#ifdef __smplos_libk
		return (void*)krealloc(ptr,size);
	#else
		uint32_t* new_alloc = __unsafe_malloc(size);
		memmove(new_alloc,ptr,header(ptr)->size >= size?size:header(ptr)->size);
		__unsafe_free(ptr);
		return new_alloc;
	#endif
}
void* __attribute__ ((malloc)) __unsafe_calloc(uint32_t num,size_t size){
	void* ret = __unsafe_malloc(num*size);
	memset(ret,0,num*size);
	return ret;
}
void* __attribute__ ((malloc)) __unsafe_valloc(uint32_t size,size_t alig){
	#ifdef __smplos_libk
		return (void*)kvalloc(size,alig);
	#else
		return __unsafe_malloc(size);
	#endif
	return 0;
}

void __unsafe_free(void* mem){
	#ifdef __smplos_libk
		return kfree(mem);
	#else
	
		if(!validate(mem)){
			return;
		}
	
		mem_t* block = header(mem);
		
		if(block->size >= heap_size || block->next != 0xAABBCCDD || block->prev != 0xAABBCCDD){
			return;
		
		}
		
		//sys_echo("Freeing\n",block->size);
		
		free_insert(block);
	#endif
}

void* __attribute__ ((malloc)) malloc(size_t size){
	lock();
	void* ret =  __unsafe_malloc(size);
	unlock();
	return ret;
}

void* __attribute__ ((malloc)) valloc(size_t size,uint32_t alig){
	lock();
	void* ret =  __unsafe_valloc(size,alig);
	unlock();
	return ret;
}
void free(void* mem){
	lock();
	__unsafe_free(mem);
	unlock();
}
void* __attribute__ ((malloc)) realloc(uint32_t* ptr,size_t size){
	lock();
	void* ret =  __unsafe_realloc(ptr,size);
	unlock();
	return ret;
}


void* __attribute__ ((malloc)) calloc(uint32_t num,size_t size){
	lock();
	void* ret =  __unsafe_calloc(num,size);
	unlock();
	return ret;
}


