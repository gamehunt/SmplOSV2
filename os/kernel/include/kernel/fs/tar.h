/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once

#include <kernel/fs/vfs.h>

struct tar_header
{
    char filename[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag[1];
};
typedef struct tar_header tar_hdr_t;

fs_node_t* tar_mount(fs_node_t* root);
uint8_t tar_umount(fs_node_t* root);
uint32_t tar_read(fs_node_t* node,uint64_t offset, uint32_t size, uint8_t* buffer);
fs_node_t* tar_seek(char* s,fs_node_t* root);
fs_node_t* tar_header2node(tar_hdr_t* hdr);
uint32_t tar_header_size(tar_hdr_t* hdr);

uint32_t init_tar();
