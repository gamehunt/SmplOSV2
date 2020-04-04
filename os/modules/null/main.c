/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/module/module.h>
#include <kernel/fs/vfs.h>

fs_node_t* null_mount(fs_node_t* root,fs_node_t* device){
	return root;
}	

uint32_t null_read(fs_node_t* node, uint64_t offs, uint32_t size, uint8_t* buffer){
	memset(buffer,0,size);
	return size;
}

uint32_t null_write(fs_node_t* node, uint64_t offs, uint32_t size, uint8_t* buffer){
	return size;
}
	
uint8_t load(){
	fs_t* nullfs = kmalloc(sizeof(fs_t));
	memset(nullfs,0,sizeof(fs_t));
	nullfs->mount = null_mount;
	nullfs->read = null_read;
	nullfs->write = null_write;
	nullfs->name = "null";
	uint32_t idx = register_fs(nullfs);
	kmount("/dev/null","",idx);
	return 0;
}
uint8_t unload(){
	return 0;
}

KERNEL_MODULE("null",load,unload,0,"");
