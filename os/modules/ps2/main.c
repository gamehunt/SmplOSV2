/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/module/module.h>
#include <kernel/misc/log.h>

#define PS2_IOPORT_DATA            0x60
#define PS2_IOPORT_STATUS          0x64
#define PS2_IOPORT_CMD             0x64

#define PS2_CMD_READ_BASE          0x20
#define PS2_CMD_WRITE_BASE         0x60

#define PS2_CMD_PORT_ENABLE        0xA8
#define PS2_CMD_PORT_DISABLE       0xA7
#define PS2_CMD_PORT_TEST          0xA9

#define PS2_CMD_SELFTEST           0xAA

#define PS2_CMD_MAIN_PORT_TEST     0xAB
#define PS2_CMD_MAIN_PORT_ENABLE   0xAD
#define PS2_CMD_MAIN_PORT_DISABLE  0xAE

#define PS2_PORT_TEST_PASSED       0x00
#define PS2_PORT_TEST_FAILED       0x01

#define PS2_SELFTEST_PASSED        0x55
#define PS2_SELFTEST_FAILED        0xFC

void ps2_wait_write(){
	while(inb(PS2_IOPORT_STATUS) & 2);
}

void ps2_wait_read(){
	while(inb(PS2_IOPORT_STATUS) & 2);
}

uint8_t ps2_read_ram(uint8_t offset){
	ps2_wait_write();
	outb(PS2_IOPORT_CMD,PS2_CMD_READ_BASE + offset);
	ps2_wait_read();
	return inb(PS2_IOPORT_DATA);
}

void ps2_write_ram(uint8_t offset, uint8_t data){
	ps2_wait_write();
	outb(PS2_IOPORT_CMD,PS2_CMD_WRITE_BASE + offset);
	ps2_wait_write();
	outb(PS2_IOPORT_DATA,data);
}

uint8_t init_ps2_controller(){
	//Disable devices
	ps2_wait_write();
	outb(PS2_IOPORT_CMD,PS2_CMD_MAIN_PORT_DISABLE);
	ps2_wait_write();
	outb(PS2_IOPORT_CMD,PS2_CMD_PORT_DISABLE);
	inb(PS2_IOPORT_DATA); //Flush output buffer
	//TODO Controller config; e.t.c
	ps2_wait_write();
	outb(PS2_IOPORT_CMD,PS2_CMD_SELFTEST);
	ps2_wait_read();
	uint8_t resp = inb(PS2_IOPORT_DATA);
	if(resp != PS2_SELFTEST_PASSED){
		kerr("PS2 Controller selftest failed\n");
		return 0;
	}
	ps2_wait_write();
	outb(PS2_IOPORT_CMD,PS2_CMD_MAIN_PORT_ENABLE);
	ps2_wait_write();
	outb(PS2_IOPORT_CMD,PS2_CMD_PORT_ENABLE);
	//TODO devices detection and reset
	return 1;
}

uint8_t load(){
	
	if(!init_ps2_controller()){
		kerr("Failed to initialize PS2 Controller\n");
		return 1;
	}
	
	//while(1);
	return 0;
}
uint8_t unload(){
	return 0;
}

KERNEL_MODULE("ps2",load,unload,0,"");
