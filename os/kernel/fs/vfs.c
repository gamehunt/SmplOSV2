#include <kernel/fs/vfs.h>

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
	node->flags = vfs_set_flag(node->flags,VFS_PRESENT);
	return node;
}

uint16_t register_fs(fs_t* fs){
	fss[fsidx] = fs;	
	fsidx++;
	return (fsidx-1);
}

static uint32_t vfs_read(fs_node_t* node, uint64_t offset, uint32_t size, uint8_t* buffer){
	memcpy(buffer,(void*)(node->inode+offset),size);
	return size;
}
static uint32_t vfs_write(fs_node_t* node, uint64_t offset, uint32_t size, uint8_t* buffer){
	memcpy((void*)(node->inode+offset),buffer,size);
	return size;
}
static fs_node_t* vfs_seek(char* name,fs_node_t* par){
	//printf("Seeking for %s from %s\n",name,par->name);
	if(!par){
		par = root;
	}
	if(!strcmp(name,"..")){
		return par;
	}
	for(int i=0;i<par->ccount;i++){
		//printf("--Got: %a (%d): %s\n",(uint32_t)par->childs[i],i,par->childs[i]->name);
		if(!strcmp(name,par->childs[i]->name)){
			return par->childs[i];
		}
	}
	return 0;
}
static fs_node_t* vfs_create(char* name,fs_node_t* par,uint8_t type){
	fs_node_t* new = allocate_fs_node();
	new->type = type;
	memcpy(new->name,name,strlen(name));
	new->parent = par;
	par->ccount++;
	par->childs = krealloc(par->childs,par->ccount*sizeof(fs_node_t*));
	par->childs[par->ccount-1] = new; //Something corrupts this array TODO: fix
	//printf("Created: %s in %s (0x%x)\n",par->childs[par->ccount-1]->name,par->name,(uint32_t)par->childs[par->ccount-1]);
	return par->childs[par->ccount-1]; 
}

static void vfs_update_node(fs_node_t* node){
	fs_node_t** newchilds = kmalloc(sizeof(fs_node_t*)*node->ccount);
	for(int i=0;i<node->ccount;i++){
		if(node->childs[i]){
			newchilds[i] = node->childs[i];
		}
	}
	kfree(node->childs);
	node->childs = newchilds;
}

static uint8_t vfs_remove(fs_node_t* n){
	uint32_t last_ccount = n->ccount;
	for(int i=0;i<last_ccount;i++){
		vfs_remove(n->childs[i]);
	}
	kfree(n->childs);
	if(n->parent){
		for(int i=0;i<n->parent->ccount;i++){
			if(!strcmp(n->name,n->parent->childs[i]->name)){
				n->parent->childs[i] = 0;
				n->parent->ccount--;
				vfs_update_node(n->parent);
				break;
			}
		}
	}
	kfree(n);
	return 1;
}

uint8_t unregister_fs(uint16_t fs){
	if(fs >= fsidx || !fss[fs]){
		kerr("Failed to unregister fs: fsid %d not registered!\n",fs);
		return 0;
	}
	fss[fs] = 0;	
	return 1;
}

void dump_node(fs_node_t* node,uint32_t tabs){
	for(int i=0;i<tabs;i++){
		printf("-");
	}
	printf("%s\n",node->name);
	for(int i=0;i<node->ccount;i++){
		dump_node(node->childs[i],tabs+1);
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
	vfs->read = &vfs_read;
	vfs->write = &vfs_write;
	vfs->create = &vfs_create;
	vfs->remove = &vfs_remove;
	vfs->seek = &vfs_seek;
	register_fs(vfs);
	root = allocate_fs_node();
	memcpy(root->name,"[fsroot]",strlen("[fsroot]"));
}

static char* canonize_absolute(char* path){
	char* npath;
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
	return npath;
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
		return substr(path,a,b);
	}
	return path;
}
fs_node_t* kseek(char* path){
	if(!strcmp(path,"/")){
		return root;
	}
	path = canonize_absolute(path);
	fs_node_t* rnode = root;	
	for(int i=0;i<path_size(path);i++){
		char* part = path_block(path,i);
		if(!rnode || !fss[rnode->fsid]->seek){
			return 0;
		}
		rnode = fss[rnode->fsid]->seek(part,rnode);
	}
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
		par = kcreate(path,0);
	}
	if(!par){
		kerr("Failed to create parent node!\n");
	}
	node = fss[par->fsid]->create(name,par,type);
	return node;
}
uint8_t kremove(char* path){
	path = canonize_absolute(path);
	fs_node_t* node = kseek(path);
	if(node){	
		if(vfs_check_flag(node->flags,VFS_MOUNTED)){
			fss[node->fsid]->remove(node);	
		}
		vfs_remove(node);
	}
}
uint32_t kread(char* path,uint64_t offset, uint32_t size, uint8_t* buffer){
	fs_node_t* node = kseek(path);
	if(node && fss[node->fsid]->read){
		return  fss[node->fsid]->read(node,offset,size,buffer);
	}
	return 0;
}
uint32_t kwrite(char* path,uint64_t offset, uint32_t size, uint8_t* buffer){
	fs_node_t* node = kseek(path);
	if(node && fss[node->fsid]->write){
		return  fss[node->fsid]->write(node,offset,size,buffer);
	}
	return 0;
}
uint8_t kmount(char* path, uint16_t fsid){
	printf("Mounting %s\n",path);
	fs_t* fs = 0;
	if(fsidx <= fsid){
		kerr("%d is not valid fs!\n",fsid);
		return 0;
	}
	fs = fss[fsid];
	if(!fs){
		kerr("Failed to mount %s: fsid %d is unregistered!\n",path,fsid);		
		return 0;
	}
	fs_node_t* mountpoint;
	if(!(mountpoint = kseek(path))){
		mountpoint = kcreate(path,0);
		if(!mountpoint){
			kerr("Failed to create mounpoint!\n",path);
			return 0;
		}
		mountpoint->fsid = fsid;
		mountpoint->flags = vfs_set_flag(mountpoint->flags,VFS_MOUNTED);
		mountpoint->flags = vfs_set_flag(mountpoint->flags,VFS_MOUNTPOINT);
	}else if(vfs_check_flag(mountpoint->flags,VFS_MOUNTED)){
		kerr("Failed to mount %s: already mounted!\n",path);		
		return 0;
	}
	if(fs->mount){
		//printf("Going into fs->mount()...\n");
		return fs->mount(mountpoint);
	}
	return mountpoint;
}
uint8_t kumount(char* path){
	fs_node_t* node = kseek(path);
	if(node && vfs_check_flag(node->flags,VFS_MOUNTPOINT)){
		if(fss[node->fsid]->umount){
			return fss[node->fsid]->umount(node);
		}
	}else if(node){
		kerr("Failed to mount %s: not mountpoint!\n",path);
	}else{
		kerr("Failed to umount %s: path doesn't exists!\n",path);
	}
	return 0;
}

uint32_t knread(fs_node_t* node,uint64_t offset, uint32_t size, uint8_t* buffer){
	if(node && fss[node->fsid]->read){
		return  fss[node->fsid]->read(node,offset,size,buffer);
	}
	return 0;
}
uint32_t knwrite(fs_node_t* node,uint64_t offset, uint32_t size, uint8_t* buffer){
	if(node && fss[node->fsid]->write){
		return  fss[node->fsid]->write(node,offset,size,buffer);
	}
	return 0;
}

uint8_t create_vfs_mapping(char* path){
	kinfo("Mapping %s\n",path);
	return kcreate(path,0);
}
