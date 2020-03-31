/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/module/module.h>
#include <kernel/misc/log.h>
#include <kernel/dev/pci.h>

#define VBE_DISPI_IOPORT_DATA  (0x01CF)
#define VBE_DISPI_IOPORT_INDEX (0x01CE)

#define VBE_DISPI_INDEX_ID          (0)
#define VBE_DISPI_INDEX_XRES        (1)
#define VBE_DISPI_INDEX_YRES        (2)
#define VBE_DISPI_INDEX_BPP         (3)
#define VBE_DISPI_INDEX_ENABLE      (4)
#define VBE_DISPI_INDEX_BANK        (5)
#define VBE_DISPI_INDEX_VIRT_WIDTH  (6)
#define VBE_DISPI_INDEX_VIRT_HEIGHT (7)
#define VBE_DISPI_INDEX_X_OFFSET    (8)
#define VBE_DISPI_INDEX_Y_OFFSET    (9)

#define VBE_DISPI_DISABLED          (0x00)
#define VBE_DISPI_ENABLED           (0x01)

#define VBE_DISPI_BPP_4  (0x04)
#define VBE_DISPI_BPP_8  (0x08)
#define VBE_DISPI_BPP_15 (0x0F)
#define VBE_DISPI_BPP_16 (0x10)
#define VBE_DISPI_BPP_24 (0x18)
#define VBE_DISPI_BPP_32 (0x20)

#define VBE_DISPI_LFB_ENABLED (0x40)
#define VBE_DISPI_NOCLEARMEM  (0x80)

#define DEFAULT_XRES 1024
#define DEFAULT_YRES 768
#define DEFAULT_BPP VBE_DISPI_BPP_32

#define VBE_IOCTL_CHANGE_RES   0x10
#define VBE_IOCTL_CHANGE_YOFFS 0x20
#define VBE_IOCTL_CHANGE_XOFFS 0x30


void qvid_write_register(uint16_t idx,uint16_t data){
	outw(VBE_DISPI_IOPORT_INDEX,idx);
	outw(VBE_DISPI_IOPORT_DATA,data);
}

uint16_t qvid_read_register(uint16_t idx){
	outw(VBE_DISPI_IOPORT_INDEX,idx);
	uint16_t data = inw(VBE_DISPI_IOPORT_DATA);
	return data;
}

void qvid_set_resolution(uint16_t x,uint16_t y,uint16_t bpp,uint8_t use_lfb,uint8_t clear_video){
	kinfo("Trying to set resolution: %d %d %a\n",x,y,bpp);
	//while(1);
	qvid_write_register(VBE_DISPI_INDEX_ENABLE,VBE_DISPI_DISABLED);
	qvid_write_register(VBE_DISPI_INDEX_XRES, x);
	qvid_write_register(VBE_DISPI_INDEX_YRES, y);
	qvid_write_register(VBE_DISPI_INDEX_BPP, bpp);
	qvid_write_register(VBE_DISPI_INDEX_VIRT_HEIGHT,4096);
	qvid_write_register(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED |
							(use_lfb ? VBE_DISPI_LFB_ENABLED : 0) |
							(clear_video ? 0 : VBE_DISPI_NOCLEARMEM));
}
uint32_t fb_ioctl(fs_node_t* node,uint32_t req,void* argp){
	uint16_t* args = (uint16_t*)argp;
	switch(req){
		case VBE_IOCTL_CHANGE_RES:
			qvid_set_resolution(args[0],args[1],args[2],1,1);
		break;
		case VBE_IOCTL_CHANGE_YOFFS:
			qvid_write_register(VBE_DISPI_INDEX_Y_OFFSET,args[0]);
		break;
		case VBE_IOCTL_CHANGE_XOFFS:
			qvid_write_register(VBE_DISPI_INDEX_X_OFFSET,args[0]);
		break;
	}
	//kinfo("IOCTL END\n");
	return 0;
}
uint8_t load(){
	pci_device_t* qvid = pci_seek_device(0x1234,0x1111);
	if(!qvid){
		kerr("BGA not found!\n");
		return 1;
	}
	qvid_write_register(VBE_DISPI_INDEX_ID,0xB0C4);
	uint16_t ver = qvid_read_register(VBE_DISPI_INDEX_ID);
	kinfo("Version : %a\n",ver);
	uint32_t* lfb = (uint32_t*)(pci_read_value(qvid,PCI_BAR0,4) & 0xFFFFFFF0);
	for(uint32_t i = 0;i<(8*1024*1024/4096);i++){
		kmpalloc((uint32_t*)((uint32_t)lfb+i*4096),(uint32_t*)((uint32_t)lfb+i*4096),0);
	}
	kinfo("LFB at %a\n",lfb);
	fs_node_t* node = kcreate("/dev/fb0",VFS_TYPE_VIRTUAL);
	node->inode = (uint32_t)lfb;
	node->ioctl = fb_ioctl;
	return 0;
}
uint8_t unload(){
	return 0;
}

KERNEL_MODULE("qvideo",load,unload,0,"");
