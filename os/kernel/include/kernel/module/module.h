#pragma once

#include <kernel/fs/vfs.h>
#include <kernel/multiboot.h>
#include <stdint.h>

struct kernel_module{
	char name[32];
	char** dependencies;
	uint8_t state;
};
typedef struct kernel_module kernel_module_t;

multiboot_module_t* get_ramdisk_module(uint32_t id);
void add_ramdisk_module(multiboot_module_t* mod);
uint32_t count_ramdisk_modules();

void add_common_module();
void count_common_modules();

void load_modules();
uint8_t load_module(fs_node_t* fsnode);
