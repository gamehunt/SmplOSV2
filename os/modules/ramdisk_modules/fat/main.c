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
	uint8_t ebpb[476];
}__attribute__((packed)) fat_bpb_t;

typedef struct{
	uint8_t __useless[2];
	uint8_t signature;
	uint32_t volume_id;
	char volume_name[11];
	char sys_ident[8];
	uint8_t __boot_code[448];
	uint8_t boot_sig[2];
}__attribute__((packed)) fat16_bpb_t;

typedef struct{
	uint32_t sectors_per_fat;
	uint16_t flags;
	uint16_t version;
	uint32_t root_cluster;
	uint16_t fsinfo_sector;
	uint16_t backup_boot;
	uint8_t __reserved[12];
	uint8_t __useless[2];
	uint8_t signature;
	uint32_t volume_id;
	char volume_name[11];
	char sys_ident[8];
	uint8_t __boot_code[420];
	uint8_t boot_sig[2];
}__attribute__((packed)) fat32_bpb_t;

fs_node_t* fat_mount(fs_node_t* root){
	
}

uint8_t load(){
	fat_default_device = kseek("/dev/sda1");
	if(!fat_default_device){
		kerr("Failed to find FAT device /dev/sda1\n");
		return 1; 
	}
	fat_bpb_t* bpb = kmalloc(512);
	knread(fat_default_device,0,1,bpb);
	if(((fat32_bpb_t*)&bpb->ebpb[0])->signature != 0x28 && ((fat32_bpb_t*)&bpb->ebpb[0])->signature != 0x29 && ((fat16_bpb_t*)&bpb->ebpb[0])->signature != 0x28 && ((fat16_bpb_t*)&bpb->ebpb[0])->signature != 0x29){
		kerr("Can't verify FAT signature!\n");
		kfree(bpb);
		return 1;
	}
	fs_t* fatfs = kmalloc(sizeof(fs_t));
	fatfs->mount = fat_mount;
	uint32_t idx = register_fs(fatfs);
	return 0;
}
uint8_t unload(){
	return 0;
}

KERNEL_MODULE("fat",load,unload,1,"ata");
