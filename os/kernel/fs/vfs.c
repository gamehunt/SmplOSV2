/*	
    Copyright (C) 2020
     
    Author: gamehunt 
*/

#include <kernel/fs/vfs.h>
#include <kernel/misc/pathutils.h>
#include <string.h>

#define MAX_FS 256

fs_t* fss[MAX_FS];
fs_node_t* root;
static uint16_t fsidx = 0; 



uint8_t vfs_set_flag(uint8_t flags,uint8_t flag){
	return flags | (1 << flag);
}
uint8_t vfs_clear_flag(uint8_t flags,uint8_t flag){
	return flags & ~(1 << flag);
}
uint8_t vfs_check_flag(uint8_t flags,uint8_t flag){
	return flags & (1 << flag);
}

fs_node_t* allocate_fs_node(){
	fs_node_t* node = kmalloc(sizeof(fs_node_t));
	memset(node,0,sizeof(fs_node_t));
	node->entry = kmalloc(sizeof(vfs_entry_t));
	memset(node->entry,0,sizeof(vfs_entry_t));
	return node;
}

uint16_t register_fs(fs_t* fs){
	fss[fsidx] = fs;	
	fsidx++;
	return (fsidx-1);
}

static uint32_t vfs_read(fs_node_t* node, uint64_t offset, uint32_t size, uint8_t* buffer){
	if(!node->inode){
		return 0;
	}
	memcpy(buffer,(void*)(node->inode+offset),size);
	return size;
}
static uint32_t vfs_write(fs_node_t* node, uint64_t offset, uint32_t size, uint8_t* buffer){
	if(!node->inode){
		return 0;
	}
	memcpy((void*)(node->inode+offset),buffer,size);
	return size;
}
static fs_node_t* vfs_seek(char* name,fs_node_t* par){
	//printf("Seeking for %s from %s\n",name,par->name);
	if(!par){
		par = root;
	}
	
	for(int i=0;i<par->entry->child_count;i++){
		//printf("--Got: %a (%d): %s\n",(uint32_t)par->entry->childs[i],i,par->entry->childs[i]->name);
		if(!strcmp(name,par->entry->childs[i]->name)){
			return par->entry->childs[i];
		}
	}
	return 0;
}
 fs_node_t* vfs_create(char* name,fs_node_t* par,uint8_t type){
	fs_node_t* new = allocate_fs_node();
	strcpy(new->name,name);
	
	new->device = 0;
	new->entry->parent = par;
	par->entry->child_count++;
	
	if(par->entry->child_count == 1){
		par->entry->childs = kmalloc(sizeof(fs_node_t*));
	}else{
		par->entry->childs = krealloc(par->entry->childs,par->entry->child_count*sizeof(fs_node_t*));
	}
	par->entry->childs[par->entry->child_count-1] = new; 
	return par->entry->childs[par->entry->child_count-1]; 
}

fs_node_t* vfs_remove(fs_node_t* node){
	if(node->fsid == 0 && node->inode){
		kfree(node->inode);
	}
	if(node->entry->child_count && validate(node->entry->childs)){
		for(uint32_t i=0;i<node->entry->child_count;i++){
			kremove(node->entry->childs[i]);
		}
		kfree(node->entry->childs);
	}
	
	if(validate(node->entry->parent)){
		fs_node_t* par = node->entry->parent;
		int zero_idx = -1;
		for(uint32_t i=0;i<par->entry->child_count;i++){
			if(!strcmp(par->entry->childs[i]->name,node->name)){
				par->entry->childs[i] = 0;
				zero_idx = i;
			}
			if(zero_idx >= 0){
				par->entry->childs[zero_idx] = par->entry->childs[zero_idx+1];
				zero_idx++;
			}
		}
		if(zero_idx){
			par->entry->child_count--;
		}
	}
	kfree(node->entry);
	kfree(node);
}

void dump_node(fs_node_t* node,uint32_t tabs){
	for(int i=0;i<tabs;i++){
		printf("-");
	}
	printf("%s %a\n",node->name,node->entry->childs);
	for(int i=0;i<node->entry->child_count;i++){
		dump_node(node->entry->childs[i],tabs+1);
	}
}

void dump_vfs(){
	kinfo("------------------------VFS DUMP------------------------\n");
	dump_node(root,0);
	kinfo("--------------------------------------------------------\n");
}


void init_vfs(){
	fs_t* vfs = kmalloc(sizeof(fs_t));
	memset(vfs,0,sizeof(fs_t));
	vfs->name = "vfs";
	vfs->read = &vfs_read;
	vfs->write = &vfs_write;
	vfs->create = &vfs_create;
	vfs->seek = &vfs_seek;
	register_fs(vfs);
	root = allocate_fs_node();
	memcpy(root->name,"[fsroot]",strlen("[fsroot]"));
	root->fsid = 0;
}

fs_node_t* kseek(char* path){
	if(!strcmp(path,"/")){
		return root;
	}
	char* npath = canonize_absolute(path);
	fs_node_t* rnode = root;	
	for(int i=0;i<path_size(npath);i++){
		char* part = path_block(npath,i);
		fs_node_t* vc = vfs_seek(part,rnode);
		if(vc){
			rnode = vc;
			continue;
		}
		if(!rnode || !rnode->fsid || !validate(fss[rnode->fsid]->seek)){
			kfree(npath);
			return 0;
		}
		rnode = fss[rnode->fsid]->seek(part,rnode);
	}
	kfree(npath);
	return rnode;
}
fs_node_t* kcreate(char* path, uint8_t type){
	path = canonize_absolute(path);
	
	char* name = path_block(path,path_size(path)-1);
	uint32_t l = strlen(name);
	path = substr(path,0,strlen(path)-l-1);
	
	fs_node_t* node = (uint32_t)0;
	fs_node_t* par = kseek(path);
	if(!par){
		par = kcreate(path,type);
	}
	if(!par){
		kerr("Failed to create parent node!\n");
	}
	if(type == VFS_TYPE_VIRTUAL || !validate(fss[par->fsid]->create)){
		node = vfs_create(name,par,type);
	}else{
		node = fss[par->fsid]->create(name,par,type);
	}
	return node;
}

fs_node_t* kmount(char* path, char* devicep,uint16_t fsid){
	if(fsidx <= fsid){
		kerr("Mount: %d is not valid fs!\n",fsid);
		return 0;
	}
	kinfo("Mounting %s to %s with %s\n",path,devicep,fss[fsid]->name);
	fs_t* fs = 0;

	fs = fss[fsid];
	if(!validate(fs)){
		kerr("Failed to mount %s: fsid %d is unregistered!\n",path,fsid);		
		return 0;
	}
	fs_node_t* mountpoint;
	//kinfo("Check\n");
	if(!(mountpoint = kseek(path))){
		//kinfo("Creating mountpoint %s...\n",path);
		mountpoint = kcreate(path,VFS_TYPE_VIRTUAL);
		if(!mountpoint){
			kerr("Failed to create mounpoint!\n");
			return 0;
		}

	}else if(vfs_check_flag(mountpoint->flags,VFS_MOUNTPOINT)){
		kerr("Failed to mount %s: already mounted!\n",path);		
		return 0;
	}else if(vfs_check_flag(mountpoint->flags,VFS_LINK)){
		kerr("Failed to mount %s: can't mount link!\n",path);
		return 0;
	}
	mountpoint->fsid = fsid;
	mountpoint->flags = vfs_set_flag(mountpoint->flags,VFS_MOUNTPOINT);
	
	fs_node_t* device = 0;
	if(strcmp(devicep,"")){
		device = kseek(devicep);
		mountpoint->device = device;
		if(!device){
			kwarn("Device %s not exists!\n",devicep);
		}
	}
	
	
	if(validate(fs->mount)){
		return fs->mount(mountpoint,device);
	}
	return mountpoint;
}


uint32_t kread(fs_node_t* node,uint64_t offset, uint32_t size, uint8_t* buffer){
	fs_node_t* real_node = node;
	if(vfs_check_flag(real_node->flags,VFS_LINK)){
		//kinfo("LINK\n");
		real_node = (fs_node_t*)node->inode;
	}
	if(validate(real_node) && validate(fss[real_node->fsid]->read)){
		return  fss[real_node->fsid]->read(real_node,offset,size,buffer);
	}
	return 0;
}
uint32_t kwrite(fs_node_t* node,uint64_t offset, uint32_t size, uint8_t* buffer){
	fs_node_t* real_node = node;
	if(vfs_check_flag(real_node->flags,VFS_LINK)){
		real_node = (fs_node_t*)node->inode;
	}
	if(validate(real_node) && validate(fss[real_node->fsid]->write)){
		return  fss[real_node->fsid]->write(real_node,offset,size,buffer);
	}
	return 0;
}


fs_dir_t* kreaddir(fs_node_t* node){
	if(vfs_check_flag(node->flags,VFS_LINK)){
		node = (fs_node_t*)node->inode;
	}
	fs_dir_t* dir = kmalloc(sizeof(fs_dir_t));
	dir->chlds = kmalloc(sizeof(fs_node_t*));
	dir->chld_cnt = 0;
	if(node->entry->child_count){
		dir->chld_cnt += node->entry->child_count;
		dir->chlds = krealloc(dir->chlds,dir->chld_cnt*sizeof(fs_node_t*));
		memcpy(dir->chlds,node->entry->childs,node->entry->child_count*sizeof(fs_node_t*));
	}
	if(validate(fss[node->fsid]->readdir)){			
		fs_dir_t* new = fss[node->fsid]->readdir(node);
		if(new){
			dir->chld_cnt += new->chld_cnt;
			dir->chlds = krealloc(dir->chlds,dir->chld_cnt*sizeof(fs_node_t*));
			memcpy(&dir->chlds[dir->chld_cnt-new->chld_cnt],new->chlds,new->chld_cnt*sizeof(fs_node_t*));
		}
	}
	return dir;
}

uint8_t klink(char* path, char* link){
	fs_node_t* node = kseek(path);
	if(!node){
		return 0;
	}
	fs_node_t* nlink = kseek(link);
	if(!nlink){
		nlink = kcreate(link,0);
	}
	if(!nlink){
		return 0;
	}
	
	nlink->flags = vfs_set_flag(nlink->flags,VFS_LINK);
	
	nlink->inode = (uint32_t)node;
	
	kinfo("Link created: %s to %s\n",link,path);
	
	return 1;
}

uint32_t kioctl(fs_node_t* node, uint32_t req, void* argp){
	if(!node){
		return 0;
	}
	if(node->ioctl){
		return node->ioctl(node,req,argp); 
	}
	if(validate(fss[node->fsid]->ioctl)){
		return fss[node->fsid]->ioctl(node,req,argp);
	}
	return 0;
}

fs_node_t* kopen(char* path){
	fs_node_t* orig = kseek(path);
	if(orig){
		fs_node_t* opened = allocate_fs_node();
		memcpy(opened,orig,sizeof(fs_node_t));
		return opened;
	}
	return 0;
}

void kclose(fs_node_t* node){
	kfree(node);
}

uint16_t ktypeid(char* name){
	for(uint16_t i = 0;i<fsidx;i++){
		if(!strcmp(fss[i]->name,name)){
			return i;
		}
	}
	return 0;
}

uint8_t kremove(fs_node_t* node){
	if(!node){
		return 1;
	}
	if(vfs_check_flag(node->flags,VFS_MOUNTPOINT)){
		kumount(node);
	}
	if(validate(fss[node->fsid]->remove)){
		fss[node->fsid]->remove(node);
	}
	if(node->fsid){
		vfs_remove(node);
	}
	kfree(node);
	return 0;
}
uint8_t kumount(fs_node_t* node){
	if(!node){
		kerr("Failed to umount %s: no such node\n");
		return 1;
	}
	if(!vfs_check_flag(node->flags,VFS_MOUNTPOINT)){
		kerr("Failed to umount %s: not mountpoint\n");
		return 1;
	}
	vfs_clear_flag(node->flags,VFS_MOUNTPOINT);
	if(validate(fss[node->fsid]->umount)){
		fss[node->fsid]->umount(node);
	}	
}
