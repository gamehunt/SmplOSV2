/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/dev/tty.h>
#include <kernel/fs/vfs.h>
#include <kernel/memory/memory.h>

static uint16_t tty_fsid;

static fs_node_t* tty_root = 0;

static uint8_t state = 0;

fs_node_t* tty_mount(fs_node_t* root,fs_node_t* device){
	return root;
}

uint32_t tty_write(fs_node_t* node, uint64_t offset, uint32_t size, uint8_t* buffer){
	for(uint32_t i=0;i<size;i++){
		terminal_putchar(buffer[i]);
	}
	return size;
}
void tty_set_state(uint8_t s){
	state = s;
}
uint8_t tty_get_state(){
	return state;
}


void init_tty(){
	
	fs_t* tty_fs = kmalloc(sizeof(fs_t));
	tty_fs->mount = &tty_mount;
	tty_fs->write = &tty_write;
		
	tty_fsid = register_fs(tty_fs);
	tty_root = kmount("/dev/tty","",tty_fsid);

	if(!tty_root){
		kerr("Failed to create TTY device!\n");
		return;
	}
	
	if(!klink("/dev/tty","/dev/stdout")){
		kerr("Failed to link /dev/stdout\n");
	}
	if(!klink("/dev/tty","/dev/stderr")){
		kerr("Failed to link /dev/stderr\n");
	}
	
	tty_set_state(TTY_ENABLE);
	
	kinfo("TTY initialized!\n");
}
