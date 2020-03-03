#include <kernel/module/module.h>
#include <kernel/misc/log.h>
#include <kernel/fs/vfs.h>

static fs_node_t* fat_default_device;

typedef struct{
	uint8_t __jmp[3];
	char oem[8];
	uint16_t bytes_per_sector;
	uint8_t sectors_per_cluster;
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

typedef struct{
	uint32_t sig1;
	uint8_t __reserved[480];
	uint32_t sig2;
	uint32_t last_free_cluster;
	uint32_t first_available_cluster;
	uint8_t __reserved1[12];
	uint32_t sig3;
}__attribute__((packed)) fat32_fsinfo_t; 

typedef struct{
	char name[11];
	uint8_t attribs;
	uint8_t reserved;
	uint8_t creation_time_sec;
	uint16_t creation_time;
	uint16_t creation_date;
	uint16_t accessed_date;
	uint16_t first_cluster_high;
	uint16_t modified_time;
	uint16_t modified_date;
	uint16_t first_cluster_low;
	uint32_t file_size;
}__attribute__((packed)) fat_dirent_t; 

typedef struct{
	uint8_t order;
	char first_chars[10];
	uint8_t attrib;
	uint8_t type;
	uint8_t checksum;
	char mid_chars[12];
	uint16_t zero;
	char end_chars[4];
}__attribute__((packed)) fat_lfe_t; 

typedef struct{
	fat_lfe_t* lfe;
	fat_dirent_t* dirent;
}fat_cluster_entry_t;

uint8_t fat_parse_entry(uint8_t* entry){
	if(entry[0] == 0){
		return 0;
	}
	if(entry[0] == 0xE5){
		return 2;
	}
	if(entry[11] == 0x0F){
		return 3;
	}
	return 1;
}

fat_cluster_entry_t* fat_parse_cluster(fat_bpb_t* bpb,uint32_t cluster,fat_cluster_entry_t* parsed_entries){
	uint32_t fat_size = ((fat32_bpb_t*)&bpb->ebpb[0])->sectors_per_fat;
	uint32_t root_dir_sectors = ((bpb->dirent_count * 32) + (bpb->bytes_per_sector - 1)) / bpb->bytes_per_sector;
	uint32_t first_data_sector = bpb->reserved_sectors + (bpb->fat_count * fat_size) + root_dir_sectors;
	
	uint32_t cluster_sector = ((cluster - 2) * bpb->sectors_per_cluster) + first_data_sector;
	
	uint8_t* entries = kmalloc(512);
	if(!parsed_entries){
		parsed_entries = kmalloc(sizeof(fat_cluster_entry_t)*32);
	}
	for(uint32_t i= 0;i<bpb->sectors_per_cluster;i++){
	
		knread(fat_default_device,cluster_sector + i,1,entries);
		uint8_t flag = 0;
		fat_dirent_t* dirent = 0;
		fat_lfe_t* lfe = 0;
		for(uint32_t j = 0; j< 512; j+=32){
			uint8_t result = fat_parse_entry(&entries[j]);
			if(!result){
				flag = 1;
				break;
			}

			if(result == 1){
				dirent = kmalloc(sizeof(fat_dirent_t));
				memcpy(dirent,&entries[j],sizeof(fat_dirent_t));
				uint32_t ncluster = dirent->first_cluster_high<<8|dirent->first_cluster_low;
			}
			if(result == 3){
				lfe = kmalloc(sizeof(fat_lfe_t));
				memcpy(lfe,&entries[j],sizeof(fat_lfe_t));
				/*kinfo("%d\n",lfe->order - 0x41);
				for(int z=0;z<10;z++){
					if(lfe->first_chars[z] != 0){
						putchar(lfe->first_chars[z]);
					}
				}
				for(int z=0;z<12;z++){
					if(lfe->mid_chars[z] != 0){
						putchar(lfe->mid_chars[z]);
					}
				}
				for(int z=0;z<4;z++){
					if(lfe->end_chars[z] != 0){
						putchar(lfe->end_chars[z]);
					}
				}
				printf("\n");*/
			}
		}
		if(flag){
			break;
		}
	//	parsed_entries[j/32] = 
	}

	kfree(entries);

	uint8_t FAT_table[bpb->bytes_per_sector];
	uint32_t fat_offset = cluster * 4;
	uint32_t  fat_sector = bpb->reserved_sectors + (fat_offset / bpb->bytes_per_sector);
	uint32_t  ent_offset = fat_offset % bpb->bytes_per_sector;
	knread(fat_default_device,fat_sector,1,&FAT_table[0]);
	uint32_t  next_cluster = *(unsigned int*)&FAT_table[ent_offset] & 0x0FFFFFFF;
	if(next_cluster >=  0x0FFFFFF8 || next_cluster == 0x0FFFFFF7){
		return;
	}
	fat_parse_cluster(bpb,next_cluster);
}

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
	fat_parse_cluster(bpb,((fat32_bpb_t*)&bpb->ebpb[0])->root_cluster);
	return 0;
}
uint8_t unload(){
	return 0;
}

KERNEL_MODULE("fat",load,unload,1,"ata");
