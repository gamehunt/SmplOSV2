#include <kernel/fs/tar.h>
#include <kernel/memory/memory.h>

unsigned int getsize(const char *in)
{
 
    unsigned int size = 0;
    unsigned int j;
    unsigned int count = 1;
 
    for (j = 11; j > 0; j--, count *= 8)
        size += ((in[j - 1] - '0') * count);
 
    return size;
 
}

fs_node_t* tar_mount(fs_node_t* root){
	tar_hdr_t** hdrs = kmalloc(sizeof(tar_hdr_t*)*count_ramdisk_modules());
	root->inode = hdrs;
	
	for(int i=0;i<count_ramdisk_modules();i++){
		unsigned int j;
		uint32_t address = ((multiboot_module_t*)get_ramdisk_module(i))->mod_start;
		for (j = 0; ; j++)
		{
 
			struct tar_header *header = (struct tar_header *)address;
			kinfo("%s\n",header);
			if (header->filename[0] == '\0')
				break;
 
			unsigned int size = getsize(header->size);
			
			hdrs[j] = header;
 
			address += ((size / 512) + 1) * 512;
 
			if (size % 512)
				address += 512;
 
		}
		root->size += j;
	}
}
uint8_t tar_umount(fs_node_t* root){
	kfree(root->inode);
}
uint32_t tar_read(fs_node_t* node,uint64_t offset, uint32_t size, uint8_t* buffer){
	tar_hdr_t* hdr = ((tar_hdr_t**)node->inode)[offset];
	if(size > getsize(hdr->size)){
		size = getsize(hdr->size);
	}
	memcpy(buffer,(uint32_t)hdr+512,size);
	return size;
}

void init_tar(){
	fs_t* fs = kmalloc(sizeof(fs_t));
	fs->mount = &tar_mount;
	fs->read = &tar_read;
	fs->umount = &tar_umount;
	uint32_t id = register_fs(fs);
	kinfo("TarFS initialized: fsid %d\n",id);
}
