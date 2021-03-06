/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/



#include <kernel/module/symtable.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/pipe.h>
#include <kernel/misc/log.h>
#include <kernel/memory/memory.h>
#include <kernel/io/io.h>
#include <kernel/dev/pci.h>
#include <kernel/dev/rtc.h>
#include <kernel/interrupts/irq.h>
#include <kernel/proc/proc.h>
#include <string.h>
#include <stdio.h>

static sym_entry_t* symtable[1024];
static uint32_t last_sym = 0;

sym_entry_t* symbol_export(char name[64],uint32_t* addr){
	sym_entry_t* entry = kmalloc(sizeof(sym_entry_t));
	memcpy(entry->name,name,64);
	entry->addr = addr;
	symtable[last_sym] = entry;
	last_sym++;
}

sym_entry_t* symbol_get(uint32_t id){
	if(id > last_sym){
		return 0;
	}
	sym_entry_t* entry = symtable[id];
	return entry;
}

sym_entry_t* symbol_seek(char name[64]){
	for(int i=0;i<last_sym;i++){
		if(!strcmp(name,symbol_get(i)->name)){
			return symbol_get(i);
		}
	}
	return 0;
}


void init_symtable(){
	symbol_export("symbol_get",&symbol_get);
	symbol_export("symbol_export",&symbol_export);
	symbol_export("symbol_seek",&symbol_seek);
	
	symbol_export("kinfo",&kinfo);
	symbol_export("kwarn",&kwarn);
	symbol_export("kerr",&kerr);
	
	symbol_export("printf",&printf);
	symbol_export("putchar",&putchar);
	
	symbol_export("kfalloc",&kfalloc);
	symbol_export("kmpalloc",&kmpalloc);
	symbol_export("knpalloc",&knpalloc);
	symbol_export("kralloc",&kralloc);
	symbol_export("kffree",&kffree);
	
	
	symbol_export("kmalloc",&kmalloc);
	symbol_export("kfree",&kfree);
	symbol_export("krealloc",&krealloc);
	
	symbol_export("pmm_allocate_dma",&pmm_allocate_dma);
	symbol_export("pmm_free_dma",&pmm_free_dma);
	
	symbol_export("validate",&validate);
	symbol_export("virtual2physical",&virtual2physical);
	
	symbol_export("outb",&outb);
	symbol_export("outw",&outw);
	symbol_export("outl",&outl);
	symbol_export("inb",&inb);
	symbol_export("inw",&inw);
	symbol_export("inl",&inl);
	
	symbol_export("allocate_fs_node",&allocate_fs_node);
	symbol_export("kmount",&kmount);
	symbol_export("kread",&kread);
	symbol_export("kwrite",&kwrite);
	symbol_export("kseek",&kseek);
	symbol_export("kcreate",&kcreate);
	symbol_export("kopen",&kopen);
	symbol_export("kclose",&kclose);
	symbol_export("ktypeid",&ktypeid);
	symbol_export("register_fs",&register_fs);
	symbol_export("vfs_set_flag",&vfs_set_flag);
	symbol_export("vfs_check_flag",&vfs_check_flag);
	symbol_export("vfs_clear_flag",&vfs_clear_flag);
	symbol_export("dump_vfs",&dump_vfs);
	
	symbol_export("pipe_create",&pipe_create);
	
	symbol_export("strcat",&strcat);
	symbol_export("strlen",&strlen);
	symbol_export("strcmp",&strcmp);
	symbol_export("strcpy",&strcpy);
	
	symbol_export("memset",&memset);
	symbol_export("memmove",&memmove);
	symbol_export("memcmp",&memcmp);
	symbol_export("memcpy",&memcpy);
	
	symbol_export("pci_seek_device",&pci_seek_device);
	symbol_export("pci_get_deviceptr",&pci_get_deviceptr);
	symbol_export("pci_get_device",&pci_get_device);
	symbol_export("pci_get_class",&pci_get_class);
	symbol_export("pci_get_subclass",&pci_get_subclass);
	symbol_export("pci_get_vendor",&pci_get_vendor);
	symbol_export("pci_read_value",&pci_read_value);
	symbol_export("pci_write_value8",&pci_write_value8);
	symbol_export("pci_write_value16",&pci_write_value16);
	symbol_export("pci_write_value32",&pci_write_value32);
	
	symbol_export("irq_set_handler",&irq_set_handler);
	symbol_export("irq_end",&irq_end);
	
	symbol_export("process_fswait_notify",&process_fswait_notify);
	
	symbol_export("rtc_current_time",&rtc_current_time);
}
