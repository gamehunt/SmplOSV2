#include <kernel/module/module.h>
#include <kernel/misc/log.h>
#include <kernel/io/io.h>

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
#define ATA_IOBASE_W_COM       8 //ATA commands       | 8bit

#define ATA_CTRL_R_ALTSTATUS  0 //no interrupts status reg | 8bit
#define ATA_CTRL_W_DEVICECTRL 0 //bus reset                | 8bit
#define ATA_CTRL_R_DRIVEADDR  1 //drive select             | 8bit
#define ATA_CTRL_CACHE_FLUSH  0xE7

#define ATA_DRIVE_PRIMARY   0xA0
#define ATA_DRIVE_SECONDARY 0xB0

#define ATA_CMD_IDENTIFY 0xEC

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



void load(){

}
void unload(){
	
}

KERNEL_MODULE("ata",load,unload,0,"");
