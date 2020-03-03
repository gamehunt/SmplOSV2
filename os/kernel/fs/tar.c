/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/fs/tar.h>
#include <kernel/memory/memory.h>

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
	node->fsid = 2;
	node->flags = 0;
	return node;
}
static uint32_t path_size(char* path){
	uint32_t blocks = 0 ;
	for(int i=0;i<strlen(path);i++){
		if(path[i] == '/'){
			blocks++;
		}
	}
	return blocks-1;
}

static char* path_block(char* path,uint32_t block){
	//printf("BLOCK\n");
	uint32_t blocks = 0 ;
	int a=-1;
	int b=-1;
	for(int i=0;i<strlen(path);i++){
		if(blocks==block && path[i] == '/'){
			if(a < 0){
				a = i+1;
			}else{
				b = i;
				break;
			}
		}else if(path[i] == '/'){
			blocks++;
		}
	}
	if(a > 0 && b > 0){
		//printf("BLOCK END\n");
		return substr(path,a,b);
	}
	//printf("BLOCK END\n");
	return path;
}
static char* canonize_absolute(char* path){
	//printf("CANONIZE\n");
	char* npath = kmalloc(sizeof(char)*256);
	uint32_t size = strlen(path);	
	if(path[0]!='/'){
		size++;
	}
	if(path[strlen(path)-1] != '/'){
		size++;
	}
	npath = kmalloc(size+2);
	memset(npath,0,size+2);
	if(path[0]!='/'){
		npath[0]='/';
	}
	strcat(npath,path);
	if(path[strlen(path)-1] != '/'){
		strcat(npath,"/");
	}
	//printf("CANONIZE END\n");
	return npath; //npath
}
fs_node_t* tar_mount(fs_node_t* root){
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
			memcpy(header->filename,path_block(canonize_absolute(header->filename),path_size(canonize_absolute(header->filename))-1));
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
void init_tar(){
	fs_t* fs = kmalloc(sizeof(fs_t));
	fs->mount = &tar_mount;
	fs->read = &tar_read;
	fs->umount = &tar_umount;
	fs->seek = &tar_seek;
	uint32_t id = register_fs(fs);
	kinfo("TarFS initialized: fsid %d\n",id);
}
