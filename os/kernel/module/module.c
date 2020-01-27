#include <kernel/module/module.h>
#include <kernel/module/elf.h>
#include <kernel/fs/vfs.h>
#include <kernel/misc/function.h>

multiboot_module_t* ramdisk_modules[64];
uint32_t last = 0;

multiboot_module_t* get_ramdisk_module(uint32_t id){
	return ramdisk_modules[id];
}
void add_ramdisk_module(multiboot_module_t* mod){
	ramdisk_modules[last] = mod;
	last++;
}
uint32_t count_ramdisk_modules(){
	return last;
}

uint8_t load_module(fs_node_t* node){
	uint8_t* buffer = kmalloc(node->size);
	if(!knread(node,0,node->size,buffer)){
		kerr("Failed to read module file\n");
	}

	if(elf_load_file(buffer)){
		elf32_hdr_t *ehdr = (elf32_hdr_t *)buffer;
		elf32_sym_t* module_header = (elf32_sym_t*)elf_get_symbol(ehdr,"__module_header");
		if(!module_header){
			kerr("Failed to find module header\n");
			return 0;
		}else{
			elf32_sect_hdr_t *target = elf_section(ehdr, module_header->st_shndx);
			int abs = (int)ehdr + module_header->st_value + target->sh_offset;
			kernel_mod_hdr_t* str = (kernel_mod_hdr_t*)abs;
			kinfo("Loading module '%s'\n",str->name);
			return str->load();
		}
	}else{
		return 0;
	}
}
