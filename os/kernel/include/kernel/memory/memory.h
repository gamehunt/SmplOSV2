/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once

#include <stdint.h>
#include <stddef.h>

#include <kernel/multiboot.h>

#include <kernel/misc/stat.h>

#define PAGE_PRESENT   0x1
#define PAGE_RW        0x2
#define PAGE_USER      0x4
#define PAGE_WTHROUGH  0x8
#define PAGE_D_NOCACHE 0x10
#define PAGE_CACHED    0x10
#define PAGE_ACCESSED  0x20
#define PAGE_DIRTY     0x40
#define PAGE_SIZE      0x80

#define KHEAP_START 0x02000000
#define KHEAP_END   0x20000000
#define KHEAP_SIZE  (KHEAP_END-KHEAP_START)

#define USER_STACK   	   0xC0000000
#define USER_HEAP    0xD0000000
#define USER_HEAP_SIZE 64*1024 //base 64 kib heap per process

#define KERNEL_PT_MAP 0xFFC00000
#define ACPICA_BASE_ADDRESS 0x40000000
#define DMA_REGION_START    0x30000000 //Physical frame
#define DMA_REGION_SIZE     1024*1024  //1MB
#define DMA_REGION_BLOCK    64*1024

#define KHEAP_GUARD_VALUE   0xED

#define KERNEL_STACK_PER_PROCESS 8*4096
#define USER_STACK_PER_PROCESS 16*4096

struct gdt_entry {
  uint16_t limit;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t access;
  uint8_t granularity;
  uint8_t base_high;
}__attribute__((packed));

struct gdt_ptr {
  uint16_t pad;
  uint16_t limit;
  uint32_t base;
}__attribute__((packed));

struct tss_entry_struct
{
   //...................................................................
   uint32_t prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
   uint32_t esp0;       // The stack pointer to load when we change to kernel mode.
   uint32_t ss0;        // The stack segment to load when we change to kernel mode.
   uint32_t esp1;       // everything below here is unusued now.. 
   //...................................................................
   uint32_t ss1;
   uint32_t esp2;
   uint32_t ss2;
   uint32_t cr3;
   uint32_t eip;
   uint32_t eflags;
   uint32_t eax;
   uint32_t ecx;
   uint32_t edx;
   uint32_t ebx;
   uint32_t esp;
   uint32_t ebp;
   uint32_t esi;
   uint32_t edi;
   uint32_t es;         
   uint32_t cs;        
   uint32_t ss;        
   uint32_t ds;        
   uint32_t fs;       
   uint32_t gs;         
   uint32_t ldt;      
   uint16_t trap;
   uint16_t iomap_base;
} __attribute__((packed));
 
typedef struct tss_entry_struct tss_entry_t;

struct mem_block{
	
	uint8_t guard;
	uint32_t size;
	struct mem_block* prev;
	struct mem_block* next;
	
}__attribute__((packed));
typedef struct mem_block mem_t;

uint32_t* kernel_page_directory;
uint32_t* current_page_directory;
extern void enable_paging();
extern void __asm_set_page_directory(uint32_t pdir);
void set_page_directory(uint32_t pdir,uint8_t phys);


void add_gdt_entry(int num,uint32_t base,uint32_t limit,uint32_t access,uint32_t gran);
void gdt_install();
extern void gdt_load();
void tss_set_kernel_stack(uintptr_t stack);

void init_pmm(multiboot_info_t *mbt);

uint32_t kfalloc();
void kffree(uint32_t frame);

void init_paging();
void init_kheap();

//allocates page with given frame or address
void kmpalloc(uint32_t addr, uint32_t frame,uint8_t flags);
//Allocates next page with addr vaddr
uint32_t* knpalloc(uint32_t vaddr);

uint32_t* kmalloc(uint32_t size);
//frees page and frame, also clears pt if needed
void kpfree(uint32_t v_addr);
//Free only mapping, not frame
void kpfree_virtual(uint32_t v_addr);
//frees memory. 
void kfree(uint32_t* addr);
//frees aligned memory.
void kvfree(uint32_t* addr);

//allocates aligned memory !! Wastes lot's of memory if alignment is large
uint32_t* kvalloc(uint32_t size,uint32_t alignment);

//reallocates memory, currently just do new allocation and copy contents of old pointer to it !!!
uint32_t* krealloc(uint32_t* ptr,uint32_t newsize);

void mem_stat();

uint8_t validate(uint32_t addr);
uint32_t virtual2physical(uint32_t v_addr);

void pmm_protect_region(uint32_t region_start,uint32_t size);
uint32_t pmm_allocate_dma();
void pmm_free_dma(uint32_t frame);

void mem_check();

void kralloc(uint32_t region_start,uint32_t region_end);
