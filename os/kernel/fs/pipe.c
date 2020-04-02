/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/fs/pipe.h>
#include <kernel/misc/log.h>
#include <kernel/proc/proc.h>


#define PIPE_IOCTL_RESIZE 0xA0
#define PIPE_IOCTL_CLEAR  0xB0
#define PIPE_IOCTL_CREATE 0xC0

typedef struct{
	uint8_t* buffer;
	uint32_t size;
	uint32_t write_ptr;
	uint32_t read_ptr;
	proc_t** waiters;
	uint32_t waiters_cnt;
	spinlock_t* lock;
}pipe_info_t;

uint32_t idx;

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
	//kinfo("Received pipe ioctl: %a\n",req);
	if(req == PIPE_IOCTL_RESIZE){
		pipe_info_t* inf = (pipe_info_t*)pipe->inode;
		uint32_t new_size = *((uint32_t*)argp);
		inf->size = new_size;
		inf->buffer = krealloc(inf->size,new_size);
	}
	if(req == PIPE_IOCTL_CLEAR){
		pipe_info_t* inf = (pipe_info_t*)pipe->inode;
		inf->write_ptr = 0;
		inf->read_ptr = 0;
		memset(inf->buffer,0,inf->size);
	}
	if(req == PIPE_IOCTL_CREATE){
		char* path = (char*)(((uint32_t*)argp)[0]);
	//	kinfo("ioctl create_pipe: %s\n",path);
		fs_node_t* pipe = kmount(path,"",idx);
		pipe_info_t* inf = (pipe_info_t*)pipe->inode;
		inf->size = ((uint32_t*)argp)[1];
		inf->buffer = krealloc(inf->buffer,inf->size);
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
	lock_spin(&inf->lock);
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
	unlock_spin(&inf->lock);
	//kinfo("Pipe readen: %d\n",read);
	return read;
}

void pipe_notify_waiters(fs_node_t* pipe){
	
	pipe_info_t* inf = (pipe_info_t*)pipe->inode;
	for(uint32_t i=0;i<inf->waiters_cnt;i++){
			process_fswait_notify(inf->waiters[i],pipe);
	}
}

uint32_t pipe_write(fs_node_t* node,uint64_t offset, uint32_t size, uint8_t* buffer){
	//kinfo("Pipe write: %d %d\n",size,buffer[0]);
	pipe_info_t* inf = (pipe_info_t*)node->inode;
	lock_spin(&inf->lock);
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
	if(write){
		pipe_notify_waiters(node);
	}
	unlock_spin(&inf->lock);
	//kinfo("Pipe written: %d\n",write);
	return write;
}

void pipe_add_waiter(fs_node_t* node,proc_t* waiter){
	//kinfo("%s: pipe_add_waiter\n",node->name);
	pipe_info_t* pipe = (pipe_info_t*)node->inode;
	pipe->waiters_cnt++;
	if(pipe->waiters_cnt == 1){
		pipe->waiters = kmalloc(sizeof(proc_t*));
	}else{
		pipe->waiters = krealloc(pipe->waiters,pipe->waiters_cnt*sizeof(proc_t*));
	}
	pipe->waiters[pipe->waiters_cnt-1] = waiter;
}

fs_node_t* pipe_create(char* path,uint32_t buffer_size){
	fs_node_t* node = kmount(path,"",idx);
	if(node){
		pipe_info_t* inf = (pipe_info_t*)node->inode;
		inf->buffer = krealloc(inf->buffer,buffer_size);
		inf->size = buffer_size;
		node->size = buffer_size;
		inf->lock = 0;
	}
}

void pipe_umount(fs_node_t* node){
	pipe_info_t* inf = (pipe_info_t*)node->inode;
	unlock_spin(inf->lock);
	kfree(inf->buffer);
	kfree(inf);
}

void init_pipe(){
	fs_t* pipefs = kmalloc(sizeof(fs_t));
	pipefs->mount = mount_pipe;
	pipefs->umount = pipe_umount;
	pipefs->ioctl = pipe_ioctl;
	pipefs->read = pipe_read;
	pipefs->write = pipe_write;
	pipefs->name = "pipe";
	idx = register_fs(pipefs);
	kmount("/dev/pipe","",idx); //master pipe
	return 0;
}

