#include <kernel/module/module.h>
#include <kernel/misc/log.h>
#include <kernel/fs/vfs.h>

fs_node_t* fat_mount(fs_node_t* root){
	
}

uint8_t load(){
	fs_t* fatfs = kmalloc(sizeof(fs_t));
	uint32_t idx = register_fs(fatfs);
	uint8_t buffer[512];
	kread("/dev/sda1",0,1,buffer);
	kinfo("%s\n",&buffer[3]);
	return 0;
}
uint8_t unload(){
	return 0;
}

KERNEL_MODULE("fat",load,unload,1,"ata");
