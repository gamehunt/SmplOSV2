#include <kernel/module/symtable.h>
#include <kernel/fs/vfs.h>
#include <kernel/misc/log.h>
#include <kernel/memory/memory.h>
#include <kernel/io/io.h>

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
	symbol_export("kmalloc",&kmalloc);
	symbol_export("kfree",&kfree);
	symbol_export("krealloc",&krealloc);
	symbol_export("outb",&outb);
	symbol_export("outw",&outw);
	symbol_export("outl",&outl);
	symbol_export("inb",&inb);
	symbol_export("inw",&inw);
	symbol_export("inl",&inl);
}
