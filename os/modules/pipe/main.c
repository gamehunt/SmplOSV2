/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/module/module.h>
#include <kernel/misc/log.h>
#include <kernel/fs/vfs.h>

#define PIPE_IOCTL_RESIZE 0xA0
#define PIPE_IOCTL_CLEAR  0xB0

typedef struct{
	uint8_t* buffer;
	uint32_t size;
	uint32_t write_ptr;
	uint32_t read_ptr;
}pipe_info_t;


static inline uint32_t pipe_unread(pipe_info_t * pipe) {
	if (pipe->read_ptr == pipe->write_ptr) {
		return 0;
	}
	if (pipe->read_ptr > pipe->write_ptr) {
		return (pipe->size - pipe->read_ptr) + pipe->write_ptr;
	} else {
		return (pipe->write_ptr - pipe->read_ptr);
	}
}

static inline uint32_t pipe_available(pipe_info_t * pipe) {
	if (pipe->read_ptr == pipe->write_ptr) {
		return pipe->size - 1;
	}

	if (pipe->read_ptr > pipe->write_ptr) {
		return pipe->read_ptr - pipe->write_ptr - 1;
	} else {
		return (pipe->size - pipe->write_ptr) + pipe->read_ptr - 1;
	}
}

uint32_t pipe_ioctl(fs_node_t* pipe,uint32_t req,void* argp){
	if(req == PIPE_IOCTL_RESIZE){
		pipe_info_t* inf = (pipe_info_t*)pipe->inode;
		uint32_t new_size = *((uint32_t*)argp);
		inf->size = new_size;
		inf->buffer = krealloc(inf->size);
	}
	if(req == PIPE_IOCTL_CLEAR){
		pipe_info_t* inf = (pipe_info_t*)pipe->inode;
		inf->write_ptr = 0;
		inf->read_ptr = 0;
		memset(inf->buffer,0,inf->size);
	}
	return 0;
}

fs_node_t* mount_pipe(fs_node_t* root,fs_node_t* device){
	pipe_info_t* inf = kmalloc(sizeof(pipe_info_t));
	memset(inf,0,sizeof(pipe_info_t));
	inf->size = 128;
	inf->buffer = kmalloc(128);
	root->inode = (uint32_t)inf;
	return root;
}

uint32_t pipe_read(fs_node_t* node,uint64_t offset, uint32_t size, uint8_t* buffer){
	pipe_info_t* inf = (pipe_info_t*)node->inode;
	uint32_t read = 0;
	for(uint32_t i = 0; i<size; i++){
		if(pipe_unread(inf) == 0){
			break;
		}
		buffer[i] = inf->buffer[inf->read_ptr];
		inf->read_ptr++;
		if(inf->read_ptr == inf->size){
			inf->read_ptr = 0;
		}
		read++;
	}
	return read;
}

uint32_t pipe_write(fs_node_t* node,uint64_t offset, uint32_t size, uint8_t* buffer){
	pipe_info_t* inf = (pipe_info_t*)node->inode;
	uint32_t write = 0;
	for(uint32_t i = 0; i<size; i++){
		if(pipe_available(inf) == 0){
			break;
		}
		inf->buffer[inf->write_ptr] = buffer[i];
		inf->write_ptr++;
		if(inf->write_ptr == inf->size){
			inf->write_ptr = 0;
		}
		write++;
	}
	return write;
}

uint8_t load(){
	fs_t* pipefs = kmalloc(sizeof(fs_t));
	pipefs->mount = mount_pipe;
	pipefs->ioctl = pipe_ioctl;
	pipefs->read = pipe_read;
	pipefs->write = pipe_write;
	pipefs->name = "pipe";
	register_fs(pipefs);
	return 0;
}
uint8_t unload(){
	return 0;
}

KERNEL_MODULE("pipe",load,unload,0,"");
