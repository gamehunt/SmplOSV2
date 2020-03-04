/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once

#include <stdint.h>
#include <stddef.h>

#include <kernel/multiboot.h>

#include <kernel/misc/stat.h>

#define PMM_STATE_FREE 0
#define PMM_STATE_OCCUPIED 1

#define PAGE_PRESENT 0x1
#define PAGE_RW 0x2
#define PAGE_USER 0x4
#define PAGE_WTHROUGH 0x8
#define PAGE_D_NOCACHE 0x10
#define PAGE_CACHED 0x10
#define PAGE_ACCESSED 0x20
#define PAGE_DIRTY 0x40
#define PAGE_SIZE 0x80

#define KHEAP_SIZE 4096*4096 //16 mib heap



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

struct memory_node{
  uint32_t size;
  uint32_t addr;
  uint8_t state;
  uint32_t prev,next;
};

typedef struct memory_node memory_node_t;

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

uint32_t* kernel_page_directory;
uint32_t* current_page_directory;
extern void enable_paging();
extern void set_page_directory(uint32_t pdir);


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
void kmpalloc(uint32_t addr, uint32_t frame);
//Just allocates next page with frame given from kfalloc()
uint32_t* kpalloc();
//Allocates next page with addr vaddr
uint32_t* knpalloc(uint32_t vaddr);
//Allocates n pages, which mapped as continious
uint32_t* kcpalloc(uint32_t n);

uint32_t* kmalloc(uint32_t size);
//frees memory. 
void kfree(uint32_t* addr);

//allocates aligned memory !! Wastes lot's of memory if alignment is large
uint32_t* kvalloc(uint32_t size,uint32_t alignment);

//reallocates memory, currently just do new allocation and copy contents of old pointer to it !!!
uint32_t* krealloc(uint32_t* ptr,uint32_t newsize);

void mem_stat();

