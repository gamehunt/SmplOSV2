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

#define ATA_CMD_IDENTIFY     0xEC
#define ATA_CMD_CACHE_FLUSH  0xE7

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

typedef struct{
	uint8_t bus;
	uint8_t drive;
	uint8_t lba48;
	uint32_t lba28_blocks;
	uint64_t lba48_blocks;
	uint32_t seccount48;
	uint16_t seccount28;
}ata_device_t;

static ata_device_t* devices[4];
static uint8_t device_idx = 0;

void ata_seldrive(uint8_t bus,uint8_t drive){
	uint16_t base = bus?ATA_PORT_IOBASE_PRIMARY:ATA_PORT_IOBASE_SECONDARY;
	outb(base+ATA_IOBASE_RW_DRIVE,drive?ATA_DRIVE_MASTER:ATA_DRIVE_SLAVE);
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
			//kinfo("Fetching data...\n");
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

static uint32_t concat2(uint16_t a,uint16_t b){
	uint16_t pow = 10;
	uint32_t xe = a;
    while(b >= pow)
        pow *= 10;
    return (uint32_t)(xe * pow + b); 
}

static uint64_t concat2e(uint32_t a,uint32_t b){
	uint16_t pow = 10;
	uint64_t xe = a;
    while(b >= pow)
        pow *= 10;
    return (uint32_t)(xe * pow + b); 
}

ata_device_t* ata_create_device(uint8_t bus,uint8_t drive,uint16_t* buffer){
	ata_device_t* device = kmalloc(sizeof(ata_device_t));
	uint16_t base = bus?ATA_PORT_IOBASE_PRIMARY:ATA_PORT_IOBASE_SECONDARY;
	ata_seldrive(bus,drive);
	device->lba48 = GET_BIT(buffer[83],10);
	device->lba28_blocks = concat2(buffer[61],buffer[60]);
	uint32_t a = concat2(buffer[101],buffer[100]);
	uint32_t b = concat2(buffer[103],buffer[102]);
	device->lba48_blocks = concat2e(b,a);
	device->bus = bus;
	device->drive = drive;
	device->seccount28 = inb(base+ATA_IOBASE_RW_SECCOUNT);
	device->seccount48 = inw(base+ATA_IOBASE_RW_SECCOUNT);
	return device;
}

uint32_t ata_read_device(ata_device_t* dev,uint64_t lba,uint32_t size,uint8_t* buffer){
	if(dev->lba48){
		
	}else{
		
	}
}

uint32_t ata_write_device(ata_device_t* dev,uint64_t lba,uint32_t size,uint8_t* buffer){
	if(dev->lba48){
		
	}else{
		
	}
}

fs_node_t* ata_mount(fs_node_t* node){
	node->inode = (uint32_t*)devices[device_idx-1];
	return node;
}

uint32_t ata_read(fs_node_t* node, uint64_t offs, uint32_t size, uint8_t* buffer){
	return ata_read_device((ata_device_t*)node->inode,offs,size,buffer);
}
uint32_t ata_write(fs_node_t* node, uint64_t offs, uint32_t size, uint8_t* buffer){
	return ata_write_device((ata_device_t*)node->inode,offs,size,buffer);
}

void load(){
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
				if(device->lba48){
					kinfo("%d sectors(0=32MB)\n",device->seccount48);
				}else{
					kinfo("%d sectors(0=128kb)\n",device->seccount28);
				}
				devices[device_idx] = device;
				char path[10] = "/dev/sd";
				char sec = 'a'+device_idx;
				path[7] = sec;
				path[8] = '\0';
				kmount(path,id);
				device_idx++;
				kfree(buffer);
			}
		}
	}

}


void unload(){
	
}

KERNEL_MODULE("ata",load,unload,0,"");
