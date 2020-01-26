#include <kernel/module/symtable.h>
#include <kernel/fs/vfs.h>

sym_entry_t** symtable = SYMTABLE;
uint32_t last_sym = 0;

sym_entry_t* create_ksym(char name[64],uint32_t* addr){
	sym_entry_t* entry = kmalloc(sizeof(sym_entry_t));
	memcpy(entry->name,name,64);
	entry->addr = addr;
	symtable[last_sym] = entry;
	last_sym++;
}

sym_entry_t* get_ksym(uint32_t id){
	sym_entry_t* entry = symtable[id];
	return entry;
}

sym_entry_t* seek_ksym(char name[64]){
	for(int i=0;i<last_sym;i++){
		if(!strcmp(name,get_ksym(i)->name)){
			return get_ksym(i);
		}
	}
	return 0;
}


void init_symtable(){
	create_ksym("get_ksym",&get_ksym);
	create_ksym("seek_ksym",&seek_ksym);
	create_ksym("dump_vfs",&dump_vfs);
}
