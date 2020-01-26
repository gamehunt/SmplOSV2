#pragma once

#include <kernel/fs/vfs.h>
#include <kernel/multiboot.h>
#include <stdint.h>

struct kernel_mod_hdr{
	char* name;
	uint8_t(*load)();
	uint8_t(*unload)();
};
typedef struct kernel_mod_hdr kernel_mod_hdr_t;

struct kernel_module{
	char name[32];
	char** dependencies;
	uint8_t state;
	kernel_mod_hdr_t* hdr;
};
typedef struct kernel_module kernel_module_t;


#define KERNEL_MODULE(__name,__load,__unload) \
	kernel_mod_hdr_t __module_header = {__name,__load,__unload};\


multiboot_module_t* get_ramdisk_module(uint32_t id);
void add_ramdisk_module(multiboot_module_t* mod);
uint32_t count_ramdisk_modules();

void add_common_module();
void count_common_modules();

void load_modules();
uint8_t load_module(fs_node_t* fsnode);
