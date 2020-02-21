#include <kernel/module/module.h>
#include <kernel/misc/log.h>
#include <kernel/fs/vfs.h>

static fs_node_t* fat_default_device;

typedef struct{
	uint8_t __jmp[3];
	char oem[8];
	uint16_t bytes_per_sector;
	uint8_t sectors_per_cluser;
	uint16_t reserved_sectors;
	uint8_t fat_count;
	uint16_t dirent_count;
	uint16_t total_sectors;
	uint8_t media_desc;
	uint16_t sectors_per_fat;
	uint16_t sectors_per_track;
	uint16_t heads;
	uint32_t hidden_sectors;
	uint32_t total_sectors_large;
}__attribute__((packed)) fat_bpb_t;

fs_node_t* fat_mount(fs_node_t* root){
	
}

uint8_t load(){
	fat_default_device = kseek("/dev/sda1");
	if(!fat_default_device){
		kerr("Failed to find FAT device /dev/sda1\n");
		return 1; 
	}
	uint8_t* buffer = kmalloc(512);
	fat_bpb_t* bpb = kmalloc(sizeof(fat_bpb_t));
	knread(fat_default_device,0,1,buffer);
	memcpy(bpb,buffer,sizeof(fat_bpb_t));
	kfree(buffer);
	kinfo("%d\n",bpb->total_sectors_large);
	fs_t* fatfs = kmalloc(sizeof(fs_t));
	fatfs->mount = fat_mount;
	uint32_t idx = register_fs(fatfs);
	return 0;
}
uint8_t unload(){
	return 0;
}

KERNEL_MODULE("fat",load,unload,1,"ata");
