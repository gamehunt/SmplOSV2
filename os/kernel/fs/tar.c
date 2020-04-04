/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/fs/tar.h>
#include <kernel/memory/memory.h>
#include <kernel/misc/pathutils.h>

static uint32_t tarfs_id = 0;

static unsigned int getsize(const char *in)
{
 
    unsigned int size = 0;
    unsigned int j;
    unsigned int count = 1;
 
    for (j = 11; j > 0; j--, count *= 8)
        size += ((in[j - 1] - '0') * count);
 
    return size;
 
}
uint32_t tar_header_size(tar_hdr_t* hdr){
	return getsize(hdr->size);
}
fs_node_t* tar_header2node(tar_hdr_t* hdr){
	fs_node_t* node = allocate_fs_node();
	node->size = tar_header_size(hdr);
	memcpy(node->name,hdr->filename,strlen(hdr->filename));
	node->inode = hdr;
	node->fsid = tarfs_id;
	node->flags = 0;
	return node;
}

fs_node_t* tar_mount(fs_node_t* root,fs_node_t* device){
	tar_hdr_t** hdrs = kmalloc(sizeof(tar_hdr_t*)*module_ramdisk_count());
	root->inode = hdrs;
	
	for(int i=0;i<module_ramdisk_count();i++){
		unsigned int j;
		uint32_t address = ((multiboot_module_t*)module_ramdisk_get(i))->mod_start;
		for (j = 0; ; j++)
		{
 
			struct tar_header *header = (struct tar_header *)address;
			//kinfo("%s\n",header);
			if (header->filename[0] == '\0')
				break;
			strcpy(header->filename,path_block(canonize_absolute(header->filename),path_size(canonize_absolute(header->filename))-1));
			unsigned int size = getsize(header->size);
			
			hdrs[j] = header;
 
			address += ((size / 512) + 1) * 512;
 
			if (size % 512)
				address += 512;
 
		}
		root->size += j;
	}
	root->size--;
	return root;
}
uint8_t tar_umount(fs_node_t* root){
	kfree(root->inode);
	return 1;
}


uint32_t tar_read(fs_node_t* node,uint64_t offset, uint32_t size, uint8_t* buffer){
	//kinfo("TAR: %d\n",size);
	if(vfs_check_flag(node->flags,VFS_MOUNTPOINT)){
		
		if(offset+1 > node->size){
			return 0;
		}
		uint32_t value = (uint32_t)(((tar_hdr_t**)node->inode)[offset+1]);
		memcpy(buffer,&value,sizeof(uint32_t));
		return size;
	}else{
		if(offset + size > node->size){
			return 0;
		}
		//kinfo("HERE\n");
		tar_hdr_t* hdr = ((tar_hdr_t*)node->inode);
		memcpy(buffer,(uint32_t)hdr+(uint32_t)offset+512,size);
		//printf("%a %d %a %a %a %a\n",(uint32_t)hdr+(uint32_t)offset+512,size,buffer[0],buffer[1],buffer[2],buffer[3]);
		return size;
	}
}
fs_node_t* tar_seek(char* s,fs_node_t* root){
	if(!strcmp("ramdisk",s)){
		return 0;
	}
	for(uint32_t i=0;i<root->size;i++){
		tar_hdr_t* hdr = ((tar_hdr_t**)root->inode)[i];
		//char* path = canonize_absolute(hdr->filename);
		//char* name = path_block(path,path_size(path)-1);
		//kinfo("%s\n",name);
		if(!strcmp(hdr->filename,s)){
			fs_node_t* node = tar_header2node(hdr);
			node->inode = hdr;
			return node;
		}
	}
	return 0;
}
uint32_t init_tar(){
	fs_t* fs = kmalloc(sizeof(fs_t));
	memset(fs,0,sizeof(fs_t));
	fs->name = "tar";
	fs->mount = &tar_mount;
	fs->read = &tar_read;
	fs->seek = &tar_seek;
	tarfs_id = register_fs(fs);
	kinfo("TarFS initialized: fsid %d\n",tarfs_id);
	return tarfs_id;
}
