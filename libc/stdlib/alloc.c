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
static uint32_t max_allocation = 0;

#define VALIDATE_PTR(ptr) (USER_HEAP<=ptr && ptr<= USER_HEAP+heap_size)
#define validate VALIDATE_PTR 

static mem_t* free_list = 0;

static inline mem_t* header(void* alloc){
	return (mem_t*)((uint32_t)alloc - sizeof(mem_t));
} 
static inline void* ptr(mem_t* alloc){
	return (void*)((uint32_t)alloc + sizeof(mem_t));
} 

void mem_check(){
	sys_echo("---------MEMCHECK------------\n");
	mem_t* block = USER_HEAP;
	mem_t* next_block;
	uint32_t errors = 0;
	while(validate(block) && block < heap_start){
		next_block = (uint32_t)block + sizeof(mem_t) + block->size;
		if(block->guard != KHEAP_GUARD_VALUE || block->size > max_allocation){
			sys_echo("\t [B] -- block: %p/%p [%p %d %p %p] - invalid guard/size\n",block,heap_start,block->guard,block->size,block->prev,block->next);
			errors++;
		}else if(next_block < heap_start && (next_block->guard != KHEAP_GUARD_VALUE || next_block->size > max_allocation)){
			sys_echo("\t [O] -- block: %p/%p [%p %d %p %p] - possible overflow\n",block,heap_start,block->guard,block->size,block->prev,block->next);
			errors++;
		}
		if((block->next && block->next != 0xAABBCCDD && !validate(block->next)) || (block->prev && block->prev != 0xAABBCCDD && !validate(block->prev))){
			sys_echo("\t [B] -- block: %p/%p [%p %d %p %p] - Invalid references\n",block,heap_start,block->guard,block->size,block->prev,block->next);
			errors++;
		}
		if((block->next != 0xAABBCCDD && block->prev != 0xAABBCCDD) && ( (block->next && (block->next < block)) || (block->prev && (block->prev > block)))){
			sys_echo("\t [C] -- block: %p/%p [%p %d %p %p] - Possible cycle\n",block,heap_start,block->guard,block->size,block->prev,block->next);
			errors++;
		}
		if(!block->size){
			sys_echo("Found null size block!\n");
			errors++;
			break;
		}
		block = next_block;
	}
	sys_echo("Total errors: %d\n",errors);
	sys_echo("-----------------------------\n");
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
	newb->guard = KHEAP_GUARD_VALUE;
	orig->size = size;
	//sys_echo("USER: SPLIT: %d to %d + %d + %d | REQ: %d\n",osize,orig->size,newb->size,sizeof(mem_t),size);
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
			free_remove(freeb);
			mem_t* new_b = split(freeb,size);
			if(validate(new_b)){
				free_insert(new_b);
			}
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
	sys_echo("Fixed heap start: %p\n",heap_start);
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
		if(size > max_allocation){
			max_allocation = size;
		}
		mem_t* block = free_block(size);
		if(block && block->guard == KHEAP_GUARD_VALUE && block->size == size){
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
			nblock->guard = KHEAP_GUARD_VALUE;
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
		kfree(mem);
	#else
	
		//return;
	
		if(!validate(mem)){
			return;
		}
	
		mem_t* block = header(mem);
		
		if(block->guard != KHEAP_GUARD_VALUE || block->size > max_allocation || block->next != 0xAABBCCDD || block->prev != 0xAABBCCDD){
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


