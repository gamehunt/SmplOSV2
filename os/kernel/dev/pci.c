/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include<kernel/dev/pci.h>

uint16_t __pci_dev = 0;


pci_device_t* pci_get_deviceptr(uint16_t i){
	if(i >= __pci_dev){
		return 0;
	}
	return pci_devices[i];
}

static inline uint32_t pci_get_addr(pci_device_t* dev, int field) {
	return 0x80000000 | (dev->bus << 16) | (dev->slot << 11) | (dev->func << 8) | ((field) & 0xFC);
}

uint16_t pci_last_device(){
	return __pci_dev;
}

void add_pci_dev(pci_device_t* dev){
	pci_devices[__pci_dev] = dev;
	__pci_dev++;
}

uint16_t pci_get_vendor(pci_device_t* dev){
	//kinfo("%d %d %d\n",dev->bus,dev->slot,dev->func);
	return pci_read_value(dev,PCI_VENDOR,2);
}

uint16_t pci_get_device(pci_device_t* dev){
	return pci_read_value(dev,PCI_DEVICE,2);
}

uint16_t pci_get_class(pci_device_t* dev){
	return pci_read_value(dev,PCI_CLASS,1) << 8;
}

uint16_t pci_get_subclass(pci_device_t* dev){
	return pci_read_value(dev,PCI_SUBCLASS,1) >> 8;
}


void pci_write_value(pci_device_t* dev,int field,uint32_t val){
	outl(PCI_ADDRESS_PORT, pci_get_addr(dev, field));
	outl(PCI_VALUE_PORT, val);
}

uint32_t pci_read_value(pci_device_t* dev, int field, int size){
	outl(PCI_ADDRESS_PORT, pci_get_addr(dev, field));
	if (size == 4) {
		return inl(PCI_VALUE_PORT);
	} else if (size == 2) {
		return inw(PCI_VALUE_PORT + (field & 2));
	} else if (size == 1) {
		return inb(PCI_VALUE_PORT + (field & 3));
	}
	return 0xFFFF;
}

void pci_probe(){
     kinfo("Starting PCI probe\n");
     for(uint16_t bus = 0; bus < 256; bus++) {
         for(uint8_t slot = 0; slot < 32; slot++) {
             for(uint8_t function = 0; function < 8; function++)
            {
					pci_device_t* dev = kmalloc(sizeof(pci_device_t));
                    dev->bus = bus;
                    dev->slot = slot;
                    dev->func = function;
                    uint16_t vendor = pci_get_vendor(dev);
                    if(vendor == 0xffff){
						kfree(dev);
					}else{
                    	uint16_t device = pci_get_device(dev);
                    	kinfo("\tvendor: 0x%x device: 0x%x\taddr: 0x%x\n",vendor, device, dev);
                    	add_pci_dev(dev);
					}
            }
         }
     }
    kinfo("Total devices: %d\n",__pci_dev);
}

void init_pci(){
	//while(1);
	pci_probe();	
}
