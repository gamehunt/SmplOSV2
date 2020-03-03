
/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/memory/memory.h>
//#include <kernel/debug/debug.h>
struct gdt_entry gdt[6];
struct gdt_ptr gp;
tss_entry_t tss_entry;

void add_gdt_entry(int num,uint32_t base,uint32_t limit,uint32_t access,uint32_t gran) {
  gdt[num].base_low = base;
  gdt[num].base_middle = (base >> 16);
  gdt[num].base_high = (base >> 24);
  gdt[num].limit = limit;
  gdt[num].granularity = gran;
  gdt[num].access = access;
}

static void write_tss(int32_t num, uint16_t ss0, uint32_t esp0) {
	tss_entry_t * tss = &tss_entry;
	uintptr_t base = (uintptr_t)tss;
	uintptr_t limit = base + sizeof *tss;

	add_gdt_entry(num, base, limit, 0xE9, 0x00);

	memset(tss, 0x0, sizeof *tss);

	tss->ss0 = ss0;
	tss->esp0 = esp0;
	tss->cs = 0x0b;
	tss->ss = 0x13;
	tss->ds = 0x13;
	tss->es = 0x13;
	tss->fs = 0x13;
	tss->gs = 0x13;

	tss->iomap_base = sizeof *tss;
}

void gdt_install() {
  
  gp.limit = (sizeof(struct gdt_entry) * 6) - 1;
  gp.base = (uint32_t)&gdt;
  add_gdt_entry(0, 0, 0, 0, 0);//0x0
  add_gdt_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);//0x8
  add_gdt_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);//0x10
  add_gdt_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);//0x18
  add_gdt_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);//0x20
  //add_gdt_entry(5, 0, 0xFFFFFFFF, 0x9A, 0x0F);//0x28
  write_tss(5, 0x10, 0x0);//0x28
  gdt_load();
  tss_flush();
}

void tss_set_kernel_stack(uintptr_t stack) {
	tss_entry.esp0 = stack;
	//tss_flush();
}
