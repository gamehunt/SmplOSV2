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
		//further load
	}else{
		kerr("Failed to load elf file\n");
		return 0;
	}
}
