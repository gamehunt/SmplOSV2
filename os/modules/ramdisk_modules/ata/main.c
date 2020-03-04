/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/
#include <kernel/module/module.h>
#include <kernel/misc/log.h>
#include <kernel/io/io.h>
#include <kernel/misc/bits.h>
#include <string.h>

#define ATA_PORT_IOBASE_PRIMARY   0x1F0
#define ATA_PORT_IOBASE_SECONDARY 0x170
#define ATA_PORT_CTRL_PRIMARY     0x3F6 
#define ATA_PORT_CTRL_SECONDARY   0x376 

#define ATA_IOBASE_RW_DATA     0 //RW PIO data        | 16bit
#define ATA_IOBASE_R_ERR       1 //Last error         | 8/16bit
#define ATA_IOBASE_W_FEAT      1 //Features           | 8/16bit
#define ATA_IOBASE_RW_SECCOUNT 2 //Numbers of sectors | 8/16bit
#define ATA_IOBASE_RW_LBALOW   3 //CHS/LBA spec.      | 8/16bit
#define ATA_IOBASE_RW_LBAMID   4 //Sector addr.       | 8/16bit
#define ATA_IOBASE_RW_LBAHIGH  5 //Sector addr.		 | 8/16bit
#define ATA_IOBASE_RW_DRIVE    6 //Drive selection    | 8bit
#define ATA_IOBASE_R_STATUS    7 //Current status     | 8bit
#define ATA_IOBASE_W_COM       7 //ATA commands       | 8bit

#define ATA_CTRL_R_ALTSTATUS  0 //no interrupts status reg | 8bit
#define ATA_CTRL_W_DEVICECTRL 0 //bus reset                | 8bit
#define ATA_CTRL_R_DRIVEADDR  1 //drive select info        | 8bit


#define ATA_DRIVE_MASTER   0xA0
#define ATA_DRIVE_SLAVE    0xB0

#define ATA_CMD_IDENTIFY      0xEC
#define ATA_CMD_CACHE_FLUSH   0xE7
#define ATA_CMD_PIO_READ      0x20
#define ATA_CMD_PIO_WRITE     0x30
#define ATA_CMD_PIO_READ_EXT  0x24
#define ATA_CMD_PIO_WRITE_EXT 0x34

#define ATA_ERR_AMNF  0 //Address not found
#define ATA_ERR_TKZNF 1 //Track zero not found
#define ATA_ERR_ABRT  2 //Aborted
#define ATA_ERR_MCR   3 //Media change req
#define ATA_ERR_IDNF  4 //ID not found
#define ATA_ERR_MC    5 //Media changed
#define ATA_ERR_UNC   6 //Uncorrectable data error
#define ATA_ERR_BBK   7 //Bad Block detected

#define ATA_DRIVREG_DRV 4 //Select drive
#define ATA_DRIVREG_LBA 6 //CHS = 0, LBA = 1

#define ATA_STATUS_ERR  0 //Error
#define ATA_STATUS_IDX  1 //IDX = 0
#define ATA_STATUS_CORR 2 //Corrected = 0
#define ATA_STATUS_DRQ  3 //1 = ready
#define ATA_STATUS_SRV  4 //Overlapped mode req
#define ATA_STATUS_DF   5 //Drive fault
#define ATA_STATUS_RDY  6 //1 = Ready, 0 = ERR/IN OP
#define ATA_STATUS_BSY  7 //1 = Drive busy

#define ATA_DEVCTRL_NIEN 1 //Disable interrupts
#define ATA_DEVCTRL_SRST 2 //Software reset, 1 -> wait -> 0 will cause reset
#define ATA_DEVCTRL_HOB  7 //read High order byte of last io value

#define ATA_DRIVADDR_DS0 0 //Select drive 0
#define ATA_DRIVADDR_DS1 1 //Select drive 1
#define ATA_DRIVADDR_HS0 2 //Selected head
#define ATA_DRIVADDR_HS1 3 //Selected head
#define ATA_DRIVADDR_HS2 4 //Selected head
#define ATA_DRIVADDR_HS3 5 //Selected head
#define ATA_DRIVADDR_WTG 6 //Write gate, 0 = write in progress
#define ATA_DRIVADDR_RES 7 //Reserved

//TODO DMA

typedef struct{
	uint16_t flags;
	uint16_t __pad[9];
	char     serial[20];
	uint16_t __pad1[3];
	char     firmware[8];
	char     model[40];
	uint16_t sectors_per_int;
	uint16_t __pad2;
	uint16_t capabilities[2];
	uint16_t __pad3[2];
	uint16_t valid_ext_data;
	uint16_t __pad4[5];
	uint16_t size_of_rw_mult;
	uint32_t lba28_sectors;
	uint16_t __pad5[38];
	uint64_t lba48_sectors;
	uint16_t __pad6[152];
}__attribute__((packed)) ata_device_info_t;

typedef struct{
	uint8_t  bootable;
	uint8_t __pad[3];
	uint8_t type;
	uint8_t __pad1[3];
	uint32_t start;
	uint32_t size;
}__attribute__((packed)) ata_patrition_t;

typedef struct{
	uint8_t bus;
	uint8_t drive;
	ata_device_info_t* info;
	ata_patrition_t* patrition_info;
}ata_device_t;



static ata_device_t* devices[256];
static uint16_t device_idx = 0;
static uint8_t letter_idx = 0;
static uint8_t selected_drive = 0;

uint8_t ata_index(uint8_t bus,uint8_t drive){
	if(bus){
		if(drive){
			return 0;
		}else{
			return 1;
		}
	}else{
		if(drive){
			return 2;
		}else{
			return 3;
		}
	}
}

void ata_seldrive(uint8_t bus,uint8_t drive){
	uint16_t base = bus?ATA_PORT_IOBASE_PRIMARY:ATA_PORT_IOBASE_SECONDARY;
	outb(base+ATA_IOBASE_RW_DRIVE,drive?ATA_DRIVE_MASTER:ATA_DRIVE_SLAVE);
	selected_drive = ata_index(bus,drive);
}

void ata_reset(uint8_t bus){
	uint16_t base = bus?ATA_PORT_IOBASE_PRIMARY:ATA_PORT_IOBASE_SECONDARY;
	outb(base+ATA_CTRL_W_DEVICECTRL,0x20);
	inb(base+ATA_IOBASE_R_STATUS);
	outb(base+ATA_CTRL_W_DEVICECTRL,0x00);
}

uint16_t* ata_ident(uint8_t bus,uint8_t drive){
	uint16_t* buffer = kmalloc(sizeof(uint16_t)*256);
	ata_seldrive(bus,drive);
	
	uint16_t base = bus?ATA_PORT_IOBASE_PRIMARY:ATA_PORT_IOBASE_SECONDARY;
	outb(base+ATA_IOBASE_RW_SECCOUNT,0);
	outb(base+ATA_IOBASE_RW_LBALOW,0);
	outb(base+ATA_IOBASE_RW_LBAMID,0);
	outb(base+ATA_IOBASE_RW_LBAHIGH,0);
	outb(base+ATA_IOBASE_W_COM,ATA_CMD_IDENTIFY);
	uint8_t status = inb(base+ATA_IOBASE_R_STATUS);

	if(status){
		while(GET_BIT(status,ATA_STATUS_BSY)){
			status = inb(base+ATA_IOBASE_R_STATUS);
		}
		if(inb(base+ATA_IOBASE_RW_LBAMID) || inb(base+ATA_IOBASE_RW_LBAHIGH)){
			    kfree(buffer);
				return 0;
		} 
		uint8_t err = 0;
		while(!GET_BIT(status,ATA_STATUS_DRQ) && !GET_BIT(status,ATA_STATUS_ERR) && !GET_BIT(status,ATA_STATUS_DF)){
				status = inb(base + ATA_IOBASE_R_STATUS);
			//	kinfo("%b %b %b %b\n",status,GET_BIT(status,ATA_STATUS_BSY),GET_BIT(status,ATA_STATUS_ERR),GET_BIT(status,ATA_STATUS_RDY));
		}
		if(!GET_BIT(status,ATA_STATUS_ERR) && !GET_BIT(status,ATA_STATUS_DF)){
			for(uint16_t i=0;i<256;i++){
				buffer[i] = inw(base+ATA_IOBASE_RW_DATA);
			}
			return buffer;
		}else{
			kfree(buffer);
			return 0;
		}
	}else{
		kfree(buffer);
		return 0;
	}
}

ata_device_t* ata_create_device(uint8_t bus,uint8_t drive,uint16_t* buffer){
	ata_device_info_t* device_info = kmalloc(sizeof(ata_device_info_t));
	device_info = (ata_device_info_t*)buffer;
	ata_device_t* device = kmalloc(sizeof(ata_device_t));
	device->bus = bus;
	device->drive = drive;
	device->info = device_info;
	device->patrition_info = 0;
	return device;
}

void ata_delay(ata_device_t* device){
	uint16_t base = device->bus?ATA_PORT_IOBASE_PRIMARY:ATA_PORT_IOBASE_SECONDARY;
	if(selected_drive != ata_index(device->bus,device->drive)){
		ata_seldrive(device->bus,device->drive);
	}
	for(int i = 0;i < 4; i++)
		inb(base + ATA_IOBASE_R_STATUS);
}

uint8_t ata_poll(ata_device_t* device){
	uint16_t base = device->bus?ATA_PORT_IOBASE_PRIMARY:ATA_PORT_IOBASE_SECONDARY;
	if(selected_drive != ata_index(device->bus,device->drive)){
		ata_seldrive(device->bus,device->drive);
	}
	while(1){
		uint8_t status = inb(base + ATA_IOBASE_R_STATUS);
		uint8_t bsy = GET_BIT(status,ATA_STATUS_BSY);
		uint8_t drq = GET_BIT(status,ATA_STATUS_DRQ);
		uint8_t err = GET_BIT(status,ATA_STATUS_ERR);
		uint8_t df  = GET_BIT(status,ATA_STATUS_DF);
		//kinfo("%d %d %d %d\n",bsy,drq,err,df);
		if(err || df){
			return 0;
		}
		if(!bsy && drq){
			return 1;
		}
	}
}

uint16_t ata_read_sector(ata_device_t* dev,uint64_t lba,uint16_t* buffer){
	if(dev->patrition_info){
		lba += dev->patrition_info->start;
	}
	uint16_t base = dev->bus?ATA_PORT_IOBASE_PRIMARY:ATA_PORT_IOBASE_SECONDARY;
	if(selected_drive != ata_index(dev->bus,dev->drive)){
		ata_seldrive(dev->bus,dev->drive);
	}
	//kinfo("%d\n",lba);
	if(dev->info->lba48_sectors > 0){
		outb(base + ATA_IOBASE_RW_DRIVE,dev->drive?0x40:0x50);
		outb(base + ATA_IOBASE_RW_SECCOUNT, 0);
		outb(base + ATA_IOBASE_RW_LBALOW, (unsigned char)lba>>24);
		outb(base + ATA_IOBASE_RW_LBAMID, (unsigned char)(lba>>32));
		outb(base + ATA_IOBASE_RW_LBAHIGH, (unsigned char)(lba>>40));
		outb(base + ATA_IOBASE_RW_SECCOUNT, 1);
		outb(base + ATA_IOBASE_RW_LBALOW, (unsigned char)lba);
		outb(base + ATA_IOBASE_RW_LBAMID, (unsigned char)(lba>>8));
		outb(base + ATA_IOBASE_RW_LBAHIGH, (unsigned char)(lba>>16));
		outb(base + ATA_IOBASE_W_COM, ATA_CMD_PIO_READ_EXT);
		outb(base + ATA_IOBASE_W_COM, ATA_CMD_CACHE_FLUSH);
	}else{
		uint32_t slba = (uint32_t)lba;

		outb(base + ATA_IOBASE_RW_DRIVE,0xE0 | (!dev->drive << 4) | (slba >> 24) & 0x0F);
		outb(base + ATA_IOBASE_RW_SECCOUNT, 1);
		outb(base + ATA_IOBASE_RW_LBALOW, (unsigned char)slba);
		outb(base + ATA_IOBASE_RW_LBAMID, (unsigned char)(slba>>8));
		outb(base + ATA_IOBASE_RW_LBAHIGH, (unsigned char)(slba>>16));
		outb(base + ATA_IOBASE_W_COM, ATA_CMD_PIO_READ);
		outb(base + ATA_IOBASE_W_COM, ATA_CMD_CACHE_FLUSH);
	}
	ata_delay(dev);
	if(ata_poll(dev)){	
		for(uint16_t i=0;i<256;i++){		
			buffer[i] = inw(base + ATA_IOBASE_RW_DATA);
		}
		return 512;
	}else{
		return 0;
	}
}

uint16_t ata_write_sector(ata_device_t* dev,uint64_t lba,uint16_t* buffer){
	if(dev->patrition_info){
		lba += dev->patrition_info->start;
	}
	uint16_t base = dev->bus?ATA_PORT_IOBASE_PRIMARY:ATA_PORT_IOBASE_SECONDARY;
	if(selected_drive != ata_index(dev->bus,dev->drive)){
		ata_seldrive(dev->bus,dev->drive);
	}
	
	if(dev->info->lba48_sectors > 0){
		outb(base + ATA_IOBASE_RW_DRIVE,dev->drive?0x40:0x50);
		outb(base + ATA_IOBASE_RW_SECCOUNT, 0);
		outb(base + ATA_IOBASE_RW_LBALOW, (unsigned char)lba>>24);
		outb(base + ATA_IOBASE_RW_LBAMID, (unsigned char)(lba>>32));
		outb(base + ATA_IOBASE_RW_LBAHIGH, (unsigned char)(lba>>40));
		outb(base + ATA_IOBASE_RW_SECCOUNT, 1);
		outb(base + ATA_IOBASE_RW_LBALOW, (unsigned char)lba);
		outb(base + ATA_IOBASE_RW_LBAMID, (unsigned char)(lba>>8));
		outb(base + ATA_IOBASE_RW_LBAHIGH, (unsigned char)(lba>>16));
		outb(base + ATA_IOBASE_W_COM, ATA_CMD_PIO_WRITE_EXT);
		outb(base + ATA_IOBASE_W_COM, ATA_CMD_CACHE_FLUSH);
	}else{
		uint32_t slba = (uint32_t)lba;

		outb(base + ATA_IOBASE_RW_DRIVE,0xE0 | (!dev->drive << 4) | (slba >> 24) & 0x0F);
		outb(base + ATA_IOBASE_RW_SECCOUNT, 1);
		outb(base + ATA_IOBASE_RW_LBALOW, (unsigned char)slba);
		outb(base + ATA_IOBASE_RW_LBAMID, (unsigned char)(slba>>8));
		outb(base + ATA_IOBASE_RW_LBAHIGH, (unsigned char)(slba>>16));
		outb(base + ATA_IOBASE_W_COM, ATA_CMD_PIO_WRITE);
		outb(base + ATA_IOBASE_W_COM, ATA_CMD_CACHE_FLUSH);
	}
	ata_delay(dev);
	if(ata_poll(dev)){
		for(uint16_t i=0;i<256;i++){
			outb(base + ATA_IOBASE_RW_DATA,buffer[i]);
		}
		return 512;
	}else{
		return 0;
	}
}

uint32_t ata_read_device(ata_device_t* dev,uint64_t lba,uint32_t size,uint8_t* buffer){
	uint32_t readen = size;
	for(uint32_t i = 0;i<size;i++){
		if(!ata_read_sector(dev,lba + i,(uint16_t*)((uint32_t)buffer+i*512))){
			kwarn("Failed to read sector!");
			readen -= 1;
		}
	}
	
	return readen;
}

uint32_t ata_write_device(ata_device_t* dev,uint64_t lba,uint32_t size,uint8_t* buffer){
	uint32_t writen = size;
	for(uint32_t i = 0;i<size;i++){
		if(!ata_write_sector(dev,lba + i,(uint16_t*)((uint32_t)buffer+i*512))){
			kwarn("Failed to write sector!");
			writen -= 1;
		}
	}
	
	return writen;
}

fs_node_t* ata_mount(fs_node_t* node){
	node->inode = (uint32_t)devices[device_idx];
	return node;
}

uint32_t ata_read(fs_node_t* node, uint64_t offs, uint32_t size, uint8_t* buffer){
	return ata_read_device((ata_device_t*)node->inode,offs,size,buffer);
}
uint32_t ata_write(fs_node_t* node, uint64_t offs, uint32_t size, uint8_t* buffer){
	return ata_write_device((ata_device_t*)node->inode,offs,size,buffer);
}

uint8_t ata_check_gpt(uint8_t buffer[512]){
	char check[] = "EFI PART";
	for(int i=0;i<8;i++){
		if(check[i] != buffer[i]){
			return 0;
		}
	}
	return 1;
}

uint8_t load(){
	fs_t* atafs = kmalloc(sizeof(fs_t));
	atafs->mount = &ata_mount;
	atafs->read = &ata_read;
	atafs->write = &ata_write;
	uint32_t id = register_fs(atafs);
	kinfo("Searching for ATA devices\n");
	for(uint8_t i = 0;i<=1;i++){
		for(uint8_t j = 0;j<=1;j++){
			uint16_t* buffer = ata_ident(i,j);
			if(buffer){
				kinfo("Found ATA device: %s, %s\n",i?"Primary bus":"Secondary bus",j?"Master":"Slave");
				ata_device_t* device = ata_create_device(i,j,buffer);
				if(device->info->lba48_sectors > 0){
					kinfo("%d sectors(0=32MB)\n",device->info->lba48_sectors);
				}else{
					kinfo("%d sectors(0=128kb)\n",device->info->lba28_sectors);
				}
				devices[device_idx] = device;
				char path[11] = "/dev/sd";
				char sec = 'a'+letter_idx;
				path[7] = sec;
				path[8] = '\0';
				kmount(path,id);
				letter_idx++;
				device_idx++;
				uint8_t buffer1[512];
				ata_read_device(device,1,1,buffer1);
				if(ata_check_gpt(buffer1)){
					kinfo("GPT patrition table\n");
				}else{
					kinfo("MBR patrition table\n");
					ata_read_device(device,0,1,buffer1);
					for(uint8_t z = 0;z<4;z++){
						ata_patrition_t* patrition = kmalloc(sizeof(ata_patrition_t));
						memcpy(patrition,&buffer1[0x1BE + z*16],sizeof(ata_patrition_t));
						if(patrition->size > 0){
							kinfo("Patrition found: %a - %a\n",patrition->start,patrition->start+patrition->size);
							ata_device_t* device_patr = ata_create_device(i,j,buffer);
							device_patr->patrition_info = patrition;
							devices[device_idx] = device_patr;
							path[8] = '1' + z;
							path[9] = '\0';
							kmount(path,id);
							device_idx++;
						}
					}
				}
			}
		}
	}
	return 0;
}


uint8_t unload(){
	return 0;
}

KERNEL_MODULE("ata",load,unload,0,"");
