/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once
#include <stdint.h>

#define MAX_FS 65535

#define VFS_PRESENT    0x1
#define VFS_MOUNTED    0x2 //Idk why...
#define VFS_MOUNTPOINT 0x2 //
#define VFS_LINK       0x4

struct fs_node{
	char name[128];	
	uint32_t size;
	uint32_t inode;
	uint8_t flags;
	uint8_t type;
	struct fs_node* parent;
	struct fs_node* device;
	struct fs_node** childs;
	uint32_t ccount;
	uint16_t fsid;
	uint32_t(*ioctl)(struct fs_node*,uint32_t req,void* argp);
};

typedef struct fs_node fs_node_t;

struct fs_dirent{
	uint32_t chld_cnt;
	fs_node_t** chlds;
};

typedef struct fs_dirent fs_dirent_t;

typedef struct{
	fs_node_t* (*mount)(fs_node_t* root,fs_node_t* device);
	uint8_t (*umount)(fs_node_t*);
	uint32_t (*read)(fs_node_t*, uint64_t, uint32_t, uint8_t*);
	uint32_t (*write)(fs_node_t*, uint64_t, uint32_t, uint8_t*);
	fs_node_t* (*seek)(char*,fs_node_t*);
	fs_node_t* (*create)(char*,fs_node_t*,uint8_t);
	fs_dirent_t* (*readdir)(fs_node_t*);
	uint8_t (*remove)(fs_node_t*);
	uint32_t (*ioctl)(fs_node_t*,uint32_t req, void* argp);
}fs_t;


void dump_vfs();
void init_vfs();
uint16_t register_fs(fs_t* fs);
uint8_t unregister_fs(uint16_t fs);

fs_node_t* create_vfs_mapping(char* path);

fs_node_t* allocate_fs_node();

uint8_t vfs_set_flag(uint8_t flags,uint8_t flag);
uint8_t vfs_clear_flag(uint8_t flags,uint8_t flag);
uint8_t vfs_check_flag(uint8_t flags,uint8_t flag);

fs_node_t* kseek(char* path); //returns node
fs_node_t* kopen(char* path); //returns node's copy
void       kclose(fs_node_t* node);
fs_node_t* kcreate(char* path, uint8_t type);
fs_node_t* kmount(char* path, char* device, uint16_t type);
uint8_t kremove(char* path);
uint8_t kumount();
uint32_t kread(fs_node_t* node,uint64_t offset, uint32_t size, uint8_t* buffer);
uint32_t kwrite(fs_node_t* node,uint64_t offset, uint32_t size, uint8_t* buffer);
uint8_t kremove(char* path);
fs_dirent_t* kreaddir(fs_node_t* node);
uint8_t klink(char* src,char* link);
uint32_t kioctl(fs_node_t* node, uint32_t req, void* argp);
