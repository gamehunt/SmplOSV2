/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once
#include <stdint.h>

#define VFS_DIRECTORY  0x01
#define VFS_MOUNTPOINT 0x02
#define VFS_LINK       0x04

#define VFS_TYPE_VIRTUAL 0x01
#define VFS_TYPE_REAL    0x00

struct fs_node;

typedef struct{
	uint32_t child_count;
	struct fs_node** childs;
	struct fs_node*  parent;
}vfs_entry_t;

struct fs_node{
	char name[64];	
	uint32_t owner;
	uint32_t size;
	uint32_t inode;
	uint8_t flags;
	uint8_t open_flags; //Set when node opened
	struct fs_node* device;
	vfs_entry_t*    entry;
	uint16_t fsid;
	uint32_t(*ioctl)(struct fs_node*,uint32_t req,void* argp); //We can make ioctl different for different nodes in the same fsid
};

typedef struct fs_node fs_node_t;

struct fs_dir{
	uint32_t chld_cnt;
	fs_node_t** chlds;
};
typedef struct fs_dir fs_dir_t;

typedef struct{
	const char* name;
	fs_node_t* (*mount)(fs_node_t* root,fs_node_t* device);
	uint8_t (*umount)(fs_node_t*);
	uint32_t (*read)(fs_node_t*, uint64_t, uint32_t, uint8_t*);
	uint32_t (*write)(fs_node_t*, uint64_t, uint32_t, uint8_t*);
	fs_node_t* (*seek)(char*,fs_node_t*);
	fs_node_t* (*create)(char*,fs_node_t*,uint8_t);
	fs_dir_t* (*readdir)(fs_node_t*); //returns child at index
	uint8_t (*remove)(fs_node_t*);
	uint32_t (*ioctl)(fs_node_t*,uint32_t req, void* argp);
}fs_t;

void dump_vfs();
void init_vfs();
uint16_t register_fs(fs_t* fs);
uint8_t unregister_fs(uint16_t fs);

fs_node_t* allocate_fs_node();

uint8_t vfs_set_flag(uint8_t flags,uint8_t flag);
uint8_t vfs_clear_flag(uint8_t flags,uint8_t flag);
uint8_t vfs_check_flag(uint8_t flags,uint8_t flag);

fs_node_t* kseek(char* path); //returns node
fs_node_t* kopen(char* path); //returns node's copy
void       kclose(fs_node_t* node);
fs_node_t* kcreate(char* path, uint8_t type);
fs_node_t* kmount(char* path, char* device, uint16_t type);
uint16_t ktypeid(char* type);
uint8_t kremove(fs_node_t* node);
uint8_t kumount(fs_node_t* node);
uint32_t kread(fs_node_t* node,uint64_t offset, uint32_t size, uint8_t* buffer);
uint32_t kwrite(fs_node_t* node,uint64_t offset, uint32_t size, uint8_t* buffer);
fs_dir_t* kreaddir(fs_node_t* node);
uint8_t klink(char* src,char* link);
uint32_t kioctl(fs_node_t* node, uint32_t req, void* argp);
