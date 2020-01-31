#pragma once

#include <kernel/fs/vfs.h>
#include <kernel/multiboot.h>
#include <stdint.h>

#define MODULE_ARRAY_SIZE 128

struct kernel_mod_hdr{
	char* name;
	uint8_t dep_cnt;
	char dependencies[256];
	uint8_t(*load)();
	uint8_t(*unload)();
};
typedef struct kernel_mod_hdr kernel_mod_hdr_t;

struct kernel_module{
	char name[32];
	uint8_t state;
	kernel_mod_hdr_t* hdr;
};
typedef struct kernel_module kernel_module_t;


#define KERNEL_MODULE(__name,__load,__unload,__cnt,__dependencies) \
	kernel_mod_hdr_t __module_header = {__name,__cnt,__dependencies,__load,__unload};\

multiboot_module_t* module_ramdisk_get(uint32_t id);
void module_ramdisk_add(multiboot_module_t* mod);
uint32_t module_ramdisk_count();

void module_common_add(fs_node_t* node);
uint32_t module_common_count();

void modules_load();

uint8_t module_load(fs_node_t* fsnode);
uint8_t module_check_dependencies(kernel_mod_hdr_t* kmod);
uint8_t module_is_loaded(char name[32]);
uint8_t module_try_dependencies_reload();
