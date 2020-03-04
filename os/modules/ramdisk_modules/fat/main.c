/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/


#include <kernel/module/module.h>
#include <kernel/misc/log.h>
#include <kernel/fs/vfs.h>

static uint32_t fsid = 0;

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
	fat_bpb_t* bpb;
	fat_lfe_t* lfe;
	fat_dirent_t* dirent;
}fat_cluster_entry_t;

uint8_t fat_entry_type(uint8_t* entry){
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

uint8_t fat_cmp_lfe(fat_lfe_t* lfe, char* string){
	uint16_t i = 0;
	uint16_t j = 0;
	while(string[i] != '\0'){
		char c = 0;
		if(i < 5){
			c = lfe->first_chars[j];
		}else if(i<11){
			if(i == 5){
				j = 0;
			}
			c = lfe->mid_chars[j];
		}else if(i<26){
			if(i==11){
				j == 0;
			}
			c = lfe->end_chars[j];
		}
		if(string[i] != c){
			return 0;
		}
		i++;
		j+=2;
	}
	return 1;
}

uint32_t fat_read_cluster(fs_node_t* device,fat_bpb_t* bpb,uint32_t cluster,uint8_t* buffer){
	uint32_t fat_size = ((fat32_bpb_t*)&bpb->ebpb[0])->sectors_per_fat;
	uint32_t root_dir_sectors = ((bpb->dirent_count * 32) + (bpb->bytes_per_sector - 1)) / bpb->bytes_per_sector;
	uint32_t first_data_sector = bpb->reserved_sectors + (bpb->fat_count * fat_size) + root_dir_sectors;
	
	uint32_t cluster_sector = ((cluster - 2) * bpb->sectors_per_cluster) + first_data_sector;
	if(buffer){
		for(uint32_t i= 0;i<bpb->sectors_per_cluster;i++){
			knread(device,cluster_sector + i,1,&buffer[i*bpb->bytes_per_sector]);
		}
	}
	uint8_t FAT_table[bpb->bytes_per_sector];
	uint32_t fat_offset = cluster * 4;
	uint32_t  fat_sector = bpb->reserved_sectors + (fat_offset / bpb->bytes_per_sector);
	uint32_t  ent_offset = fat_offset % bpb->bytes_per_sector;
	knread(device,fat_sector,1,&FAT_table[0]);
	uint32_t  next_cluster = *(unsigned int*)&FAT_table[ent_offset] & 0x0FFFFFFF;
	if(next_cluster >=  0x0FFFFFF8 || next_cluster == 0x0FFFFFF7){
		return 0;
	}
	return next_cluster;
}

uint32_t fat_parse_cluster(fs_node_t* device,fat_bpb_t* bpb,uint32_t cluster,fat_cluster_entry_t* parsed_entries){
	uint32_t fat_size = ((fat32_bpb_t*)&bpb->ebpb[0])->sectors_per_fat;
	uint32_t root_dir_sectors = ((bpb->dirent_count * 32) + (bpb->bytes_per_sector - 1)) / bpb->bytes_per_sector;
	uint32_t first_data_sector = bpb->reserved_sectors + (bpb->fat_count * fat_size) + root_dir_sectors;
	
	uint32_t cluster_sector = ((cluster - 2) * bpb->sectors_per_cluster) + first_data_sector;
	
	uint8_t* entries = kmalloc(bpb->bytes_per_sector);
	for(uint32_t i= 0;i<bpb->sectors_per_cluster;i++){
	
		knread(device,cluster_sector + i,1,entries);
		uint8_t flag = 0;
		fat_dirent_t* dirent = 0;
		fat_lfe_t* lfe = 0;
		for(uint32_t j = 0; j< 512; j+=32){
			uint8_t result = fat_entry_type(&entries[j]);
			if(!result){
				flag = 1;
				break;
			}

			if(result == 1){
				dirent = kmalloc(sizeof(fat_dirent_t));
				memcpy(dirent,&entries[j],sizeof(fat_dirent_t));
				//
			}
			if(result == 3){
				lfe = kmalloc(sizeof(fat_lfe_t));
				memcpy(lfe,&entries[j],sizeof(fat_lfe_t));
			}
			if(lfe && dirent){
				parsed_entries[i*16 + j/32].lfe = lfe;
				parsed_entries[i*16 + j/32].dirent = dirent;
				parsed_entries[i*16 + j/32].bpb = bpb;
				lfe = 0;
				dirent = 0;
			} 
		}
		if(flag){
			break;
		}
		
	}

	kfree(entries);

	uint8_t FAT_table[bpb->bytes_per_sector];
	uint32_t fat_offset = cluster * 4;
	uint32_t  fat_sector = bpb->reserved_sectors + (fat_offset / bpb->bytes_per_sector);
	uint32_t  ent_offset = fat_offset % bpb->bytes_per_sector;
	knread(device,fat_sector,1,&FAT_table[0]);
	uint32_t  next_cluster = *(unsigned int*)&FAT_table[ent_offset] & 0x0FFFFFFF;
	if(next_cluster >=  0x0FFFFFF8 || next_cluster == 0x0FFFFFF7){
		return 0;
	}
	return next_cluster;
}

fs_node_t* fat_mount(fs_node_t* root,fs_node_t* device){
	if(!device){
		kerr("Failed to mount node: Invalid device!\n");
		return 1; 
	}
	fat_bpb_t* bpb = kmalloc(512);
	knread(device,0,1,bpb);
	if(((fat32_bpb_t*)&bpb->ebpb[0])->signature != 0x28 && ((fat32_bpb_t*)&bpb->ebpb[0])->signature != 0x29 && ((fat16_bpb_t*)&bpb->ebpb[0])->signature != 0x28 && ((fat16_bpb_t*)&bpb->ebpb[0])->signature != 0x29){
		kerr("Can't verify FAT signature!\n");
		kfree(bpb);
		return 1;
	}
	root->device = device;
	fat_cluster_entry_t* rootent = kmalloc(sizeof(fat_cluster_entry_t));
	rootent->dirent = kmalloc(sizeof(fat_dirent_t));
	rootent->dirent->attribs = 0x10;
	memcpy(rootent->dirent->name,root->name,11);
	rootent->bpb = bpb;
	root->inode = (uint32_t)rootent;
	return root;
}

fs_node_t* fat_seek(char* path,fs_node_t* root){
	//kinfo("%s\n",path);
	fat_cluster_entry_t* ent = (fat_cluster_entry_t*)root->inode;
	uint32_t cluster = vfs_check_flag(root->flags, VFS_MOUNTPOINT)?((fat32_bpb_t*)&ent->bpb->ebpb[0])->root_cluster:(ent->dirent->name,ent->dirent->first_cluster_high<<8|ent->dirent->first_cluster_low);
	//kinfo("%d %s\n",cluster,ent->dirent->name);
	if(ent->dirent->attribs == 0x10){
		fat_cluster_entry_t* entries = kmalloc(sizeof(fat_cluster_entry_t)*16*ent->bpb->sectors_per_cluster);
		memset(entries,sizeof(fat_cluster_entry_t)*16*ent->bpb->sectors_per_cluster,0);
		
		
		do{
			cluster = fat_parse_cluster(root->device,ent->bpb,cluster,entries);
			for(uint32_t i = 0;i<16*ent->bpb->sectors_per_cluster;i++){
			//	kinfo("%a %a\n",entries[i].lfe,entries[i].dirent);
			//kinfo("HERE %d\n",i);
				if(!entries[i].dirent){
					continue;
				}
				//kinfo("%s\n",entries[i].dirent->name);
				if((!strcmp(entries[i].dirent->name,path)) || (entries[i].lfe && fat_cmp_lfe(entries[i].lfe,path))){
					
						fs_node_t* node = kmalloc(sizeof(fs_node_t));
						memcpy(node->name,entries[i].dirent->name,11);
						node->device = root->device;
						node->parent = root;
						fat_cluster_entry_t* seekent = kmalloc(sizeof(fat_cluster_entry_t));
						memcpy(seekent,&entries[i],sizeof(fat_cluster_entry_t));
						node->inode = (uint32_t)seekent;
						node->fsid = fsid;
						kfree(entries);
						return node;
				}
			}
		}while(cluster);
	}
	return 0;
}


uint32_t fat_read(fs_node_t* node,uint64_t offset, uint32_t size, uint8_t* buffer){
	fat_cluster_entry_t* node_entry = (fat_cluster_entry_t*)node->inode;
	fat_bpb_t* fat_bpb = node_entry->bpb;
	if(node_entry->dirent->attribs != 0x10 && offset+size < node_entry->dirent->file_size){
		uint32_t cluster = node_entry->dirent->first_cluster_high<<8|node_entry->dirent->first_cluster_low;
		while(offset > fat_bpb->sectors_per_cluster*fat_bpb->bytes_per_sector && cluster){
			cluster = fat_read_cluster(node->device,fat_bpb,cluster,0);
			offset -= fat_bpb->sectors_per_cluster*fat_bpb->bytes_per_sector;
		}
		if(offset + size < fat_bpb->sectors_per_cluster*fat_bpb->bytes_per_sector){
			uint8_t* bigbuff = kmalloc(fat_bpb->sectors_per_cluster*fat_bpb->bytes_per_sector);
			fat_read_cluster(node->device,fat_bpb,cluster,bigbuff);
			memcpy(buffer,&bigbuff[offset],size);
			kfree(bigbuff);
		}else{
			uint32_t delta = offset + size - fat_bpb->sectors_per_cluster*fat_bpb->bytes_per_sector;
			uint8_t* bigbuff = kmalloc(fat_bpb->sectors_per_cluster*fat_bpb->bytes_per_sector);
			fat_read_cluster(node->device,fat_bpb,cluster,bigbuff);
			memcpy(buffer,&bigbuff[offset],delta);
			cluster = fat_read_cluster(node->device,fat_bpb,cluster,0);
			fat_read_cluster(node->device,fat_bpb,cluster,bigbuff);
			memcpy(&buffer[delta],bigbuff,size-delta);
			kfree(bigbuff);
		}
		return size;
	}else{
		return 0;
	}
}

uint8_t load(){
	
	fs_t* fatfs = kmalloc(sizeof(fs_t));
	fatfs->mount = fat_mount;
	fatfs->read = fat_read;
	fatfs->seek = fat_seek;
	fsid = register_fs(fatfs);
	kinfo("FatFS idx: %d\n",fsid);
	//TEST CODE
	/*fat_cluster_entry_t* entries = kmalloc(sizeof(fat_cluster_entry_t)*16*fat_bpb->sectors_per_cluster);
	memset(entries,sizeof(fat_cluster_entry_t)*16*fat_bpb->sectors_per_cluster,0);
	fat_parse_cluster(fat_bpb,((fat32_bpb_t*)&fat_bpb->ebpb[0])->root_cluster,entries);
	for(uint32_t i = 0;i<16*fat_bpb->sectors_per_cluster;i++){
		if(entries[i].dirent){
			kinfo("%s\n",entries[i].dirent->name);
			uint32_t ncluster = entries[i].dirent->first_cluster_high<<8|entries[i].dirent->first_cluster_low;
			if(ncluster == 3){
				//kinfo("123\n");
				fat_cluster_entry_t* entries2 = kmalloc(sizeof(fat_cluster_entry_t)*16*fat_bpb->sectors_per_cluster);
				fat_parse_cluster(fat_bpb,ncluster,entries2);
				for(uint32_t j = 0;j<16*fat_bpb->sectors_per_cluster;j++){
					if(entries2[j].dirent){
						kinfo("\t%s - %d\n",entries2[j].dirent->name,entries2[j].dirent->first_cluster_high<<8|entries2[j].dirent->first_cluster_low);
					}
				}
			}
		}
	}*/
	return 0;
}
uint8_t unload(){
	return 0;
}

KERNEL_MODULE("fat",load,unload,1,"ata");
