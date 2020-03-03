/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/dev/tty.h>
#include <kernel/fs/vfs.h>
#include <kernel/memory/memory.h>

uint16_t tty_fsid;

fs_node_t* tty_root = 0;
fs_node_t* out_root;
fs_node_t* err_root;
uint32_t tty_stream = 0;
uint8_t enabled = 0;


fs_node_t* tty_mount(fs_node_t* root){
	//printf("HERE %s\n",root->name);
	return root;
}

uint32_t tty_write(fs_node_t* node, uint64_t offset, uint32_t size, uint8_t* buffer){
	for(uint32_t i=0;i<size;i++){
		terminal_putchar(buffer[i]);
	}
	return size;
}

uint8_t tty_is_enabled(){
	//return tty_root?1:0;
	return tty_root?1:0;;
}

fs_node_t* tty_get_root(){
	return tty_root;
}

void tty_set_output_stream(uint32_t stream){
	tty_stream = stream;
	if(stream == TTY_OUTPUT_STREAM_STDOUT){
		tty_root = out_root;	
	}else{
		tty_root = err_root;
	}
}

uint32_t tty_get_output_stream(){
	return tty_stream;
}


void init_tty(){
	
	fs_t* tty_fs = kmalloc(sizeof(fs_t));
	tty_fs->mount = &tty_mount;
	tty_fs->write = &tty_write;
		
	tty_fsid = register_fs(tty_fs);
	if(out_root = kmount("/dev/stdout/",tty_fsid)){
		if(!(err_root=kmount("/dev/stderr/",tty_fsid))){
			kerr("Failed to create stderr output handler\n");
		}
		tty_set_output_stream(TTY_OUTPUT_STREAM_STDOUT);
		enabled = 1;
		kinfo("TTY initialized: fsid %d\n",tty_fsid);
	}else{
		kerr("Failed to initialize tty\n");
	}
}
