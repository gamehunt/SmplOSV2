#include <kernel/module/ramdisk.h>
#include <kernel/fs/tar.h>

uint8_t ramdisk_load(){
	kinfo("Loading ramdisk\n");
	init_tar();
	fs_node_t* ramdisk_root;
	if(!(ramdisk_root = kmount("/ramdisk",2))){
		kerr("Failed to create ramdisk device\n");
		return 0;
	}
	for(uint32_t i=0;i<ramdisk_root->size;i++){
		uint32_t hdr_addr;
		knread(ramdisk_root,i,0,&hdr_addr);
		tar_hdr_t* hdr= (tar_hdr_t*)hdr_addr;
		fs_node_t* fsnode = header2node(hdr);
		if(!load_module(fsnode)){
			kerr("Failed to load ramdisk module '%s'\n",fsnode->name);
		}
	}
	kinfo("Ramdisk loaded\n");
}
