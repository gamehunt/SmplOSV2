/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#ifndef K_PCI_H
#define K_PCI_H
#include<stdint.h>
#include<kernel/io/io.h>

#define MAX_PCI_DEVICES 65536

#define PCI_VENDOR               0x00 // 2
#define PCI_DEVICE               0x02 // 2
#define PCI_COMMAND              0x04 // 2
#define PCI_STATUS               0x06 // 2
#define PCI_REVISION_ID          0x08 // 1

#define PCI_PROG_IF              0x09 // 1
#define PCI_SUBCLASS             0x0a // 1
#define PCI_CLASS                0x0b // 1
#define PCI_CACHE_LINE_SIZE      0x0c // 1
#define PCI_LATENCY_TIMER        0x0d // 1
#define PCI_HEADER_TYPE          0x0e // 1
#define PCI_BIST                 0x0f // 1
#define PCI_BAR0                 0x10 // 4
#define PCI_BAR1                 0x14 // 4
#define PCI_BAR2                 0x18 // 4
#define PCI_BAR3                 0x1C // 4
#define PCI_BAR4                 0x20 // 4
#define PCI_BAR5                 0x24 // 4

#define PCI_ADDRESS_PORT 0xCF8
#define PCI_VALUE_PORT   0xCFC
typedef struct pci_device{
	uint32_t func;
	uint32_t bus;
	uint32_t slot;
}pci_device_t;
pci_device_t* pci_devices[MAX_PCI_DEVICES];


void init_pci();

void add_pci_dev(pci_device_t* dev);

uint32_t pci_read_value(pci_device_t* dev, int field, int size);
void pci_write_value(pci_device_t* dev, int field, uint32_t value);

uint16_t pci_get_vendor(pci_device_t* dev);
uint16_t pci_get_device(pci_device_t* dev);
uint16_t pci_get_class(pci_device_t* dev);
uint16_t pci_get_subclass(pci_device_t* dev);

uint16_t pci_last_device(); 
pci_device_t* pci_get_deviceptr(uint16_t i); 
pci_device_t* pci_seek_device(uint16_t vendor,uint16_t device);
#endif
