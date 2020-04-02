/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/module/module.h>
#include <kernel/misc/log.h>
#include <kernel/misc/bits.h>
#include <kernel/global.h>

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

#define PS2_KBD_CMD_LED            0xED
#define PS2_KBD_CMD_ECHO           0xEE
#define PS2_KBD_CMD_SCNCD          0xF0
#define PS2_KBD_CMD_IDENT          0xF2
#define PS2_KBD_CMD_TYPEMAT        0xF3
#define PS2_KBD_CMD_ENABLE         0xF4
#define PS2_KBD_CMD_DISABLE        0xF5
#define PS2_KBD_CMD_DEFAULT        0xF6
#define PS2_KBD_CMD_RESEND         0xFE
#define PS2_KBD_CMD_SELFTEST       0xFF

//Scan code set 3 only
#define PS2_KBD_CMD_TYPEMAT_ALL    0xFA
#define PS2_KBD_CMD_TYPEMAT_KEY    0xFB
#define PS2_KBD_CMD_MR_KEY         0xFC
#define PS2_KBD_CMD_MAKEONLY_key   0xFD

#define PS2_KBD_RESP_ERROR1        0x00
#define PS2_KBD_RESP_ERROR2        0xFF
#define PS2_KBD_RESP_TEST_SUCC     0xAA
#define PS2_KBD_RESP_TEST_FAIL1    0xFC
#define PS2_KBD_RESP_TEST_FAIL2    0xFD
#define PS2_KBD_RESP_ECHO          0xEE
#define PS2_KBD_RESP_ACK           0xFA
#define PS2_KBD_RESP_RESEND        0xFE

static uint8_t ps2_kbd = 0xFF;
static uint8_t ps2_kbd_scncd_set = 0xFF;
static uint8_t ps2_mouse = 0xFF;

fs_node_t* kbd_device;

uint8_t ps2_wait_write(){
	uint16_t cnt = 0;
	while(GET_BIT(inb(PS2_IOPORT_STATUS),1)){
		cnt++;
		if(cnt > 1000){
			return 1;
		}
	}
	return 0;
}

uint8_t ps2_wait_read(){
	uint16_t cnt = 0;
	while(!(GET_BIT(inb(PS2_IOPORT_STATUS),0))){
		cnt++;
		if(cnt > 1000){
			return 1;
		}
	}
	return 0;
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

uint8_t ps2_device_read(){
	ps2_wait_read();
	return inb(PS2_IOPORT_DATA);
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
	outb(PS2_IOPORT_CMD,PS2_CMD_READ_BASE);
	ps2_wait_read();
	uint8_t cfg_byte = inb(PS2_IOPORT_DATA);
	cfg_byte = CLEAR_BIT(cfg_byte,0);
	cfg_byte = CLEAR_BIT(cfg_byte,1);
	cfg_byte = CLEAR_BIT(cfg_byte,6);
	uint8_t dual_port = GET_BIT(cfg_byte,5);
	kinfo("%s\n",dual_port?"Two PS2 ports detected":"One PS2 port detected");
	ps2_wait_write();
	outb(PS2_IOPORT_CMD,PS2_CMD_WRITE_BASE);
	ps2_wait_write();
	outb(PS2_IOPORT_DATA,cfg_byte);
	
	ps2_wait_write();
	outb(PS2_IOPORT_CMD,PS2_CMD_SELFTEST);
	ps2_wait_read();
	uint8_t resp = inb(PS2_IOPORT_DATA);
	if(resp != PS2_SELFTEST_PASSED){
		kerr("PS2 Controller selftest failed\n");
		return 0;
	}
	ps2_wait_write();
	outb(PS2_IOPORT_CMD,PS2_CMD_MAIN_PORT_TEST);
	ps2_wait_read();
	resp = inb(PS2_IOPORT_DATA);
	if(resp != PS2_PORT_TEST_PASSED){
		kerr("PS2 Port 1 test failed\n");
		return 1;
	}
	if(dual_port){
		ps2_wait_write();
		outb(PS2_IOPORT_CMD,PS2_CMD_PORT_TEST);
		ps2_wait_read();
		resp = inb(PS2_IOPORT_DATA);
		if(resp != PS2_PORT_TEST_PASSED){
			kwarn("PS2 Port 2 test failed\n");
		}
	}
	ps2_wait_write();
	outb(PS2_IOPORT_CMD,PS2_CMD_MAIN_PORT_ENABLE);
	if(dual_port){
		ps2_wait_write();
		outb(PS2_IOPORT_CMD,PS2_CMD_PORT_ENABLE);
	}
	cfg_byte = SET_BIT(cfg_byte,0);
	cfg_byte = SET_BIT(cfg_byte,1);
	ps2_wait_write();
	outb(PS2_IOPORT_CMD,PS2_CMD_WRITE_BASE);
	ps2_wait_write();
	outb(PS2_IOPORT_DATA,cfg_byte);
	ps2_device_send(1,0xFF);
	if(dual_port){
		ps2_device_send(0,0xFF);
	}
	ps2_map_devices();
	return 1;
}

void ps2_device_send(uint8_t main,uint8_t cmd){
	if(main){
		ps2_wait_write();
		outb(PS2_IOPORT_DATA,cmd);
	}else{
		ps2_wait_write();
		outb(PS2_IOPORT_CMD,0xD4);
		ps2_wait_write();
		outb(PS2_IOPORT_DATA,cmd);
	}
}

void ps2_map_devices(uint8_t dual){
	ps2_device_send(1,0xF5); // disable scan
	ps2_device_read();
	ps2_device_send(1,0xF2); // ident
	if(ps2_wait_read()){
		kinfo("Detected kbd on port 1\n");
		ps2_kbd = 1;
	}
	ps2_device_read();
	uint8_t resp = ps2_device_read();
	kinfo("Ident response on port 1: %a\n",resp);
	if(!ps2_wait_read()){
		ps2_device_read();
	}
	if(resp == 0x00 || resp == 0x03 || resp == 0x04){
		kinfo("Detected mouse on port 1\n");
		ps2_mouse = 1;
	}
	if(resp == 0xAB){
		kinfo("Detected kbd on port 1\n");
		ps2_kbd = 1;
	}
	if(!dual){
		return;
	}
	ps2_device_send(0,0xF5); // disable scan
	ps2_device_read();
	ps2_device_send(0,0xF2); // ident
	if(ps2_wait_read()){
		kinfo("Detected kbd on port 2\n");
		ps2_kbd = 0;
	}
	ps2_device_read();
	resp = ps2_device_read();
	if(!ps2_wait_read()){
		ps2_device_read();
	}
	kinfo("Ident response on port 2: %a\n",resp);
	if(resp == 0x00 || resp == 0x03 || resp == 0x04){
		kinfo("Detected mouse on port 2\n");
		ps2_mouse = 0;
	}
	if(resp == 0xAB){
		kinfo("Detected kbd on port 2\n");
		ps2_kbd = 0;
	}
}

void ps2_kbd_int_handler(regs_t regs){
	//kinfo("KBD\n");
	char scancode = inb(PS2_IOPORT_DATA);
	kwrite(kbd_device,0,1,&scancode);
	irq_end(ps2_kbd?1:12);
}

uint8_t init_ps2_kbd(){
	if(ps2_kbd == 0xFF){
		kinfo("Keyboard not found!\n");
		return 0;
	}
	ps2_device_send(ps2_kbd,PS2_KBD_CMD_ECHO);
	uint8_t resp = ps2_device_read();
	if(resp != PS2_KBD_RESP_ECHO){
		kwarn("Keyboard echo selftest failed!(%a)\n",resp);
	}else{
		kinfo("Keyboard echo selftest passed\n");
	}
	ps2_device_send(ps2_kbd,PS2_KBD_CMD_SCNCD);
	ps2_device_send(ps2_kbd,1);
	ps2_device_send(ps2_kbd,PS2_KBD_CMD_SCNCD);
	ps2_device_send(ps2_kbd,0);
	resp = ps2_device_read();
	if(resp == 0xFA){
		do{
			resp = ps2_device_read();
		}while(resp == 0xFA);
		kinfo("Keyboard uses %a scancode set\n",resp);
		ps2_kbd_scncd_set = resp;
	}else{
		kinfo("Failed to get keyboard scancode set\n");
	}
	kbd_device = kmount("/dev/kbd","",ktypeid("pipe"));
	irq_set_handler(ps2_kbd?1:12,ps2_kbd_int_handler);
	ps2_device_send(ps2_kbd,PS2_KBD_CMD_ENABLE);
	//while(1);
	return 1;
}

uint8_t init_ps2_mouse(){
	if(ps2_mouse == 0xFF){
		kinfo("Mouse not found!\n");
		return 0;
	}
	//TODO
	return 1;
}



uint8_t load(){
	
	if(!init_ps2_controller()){
		kerr("Failed to initialize PS2 Controller\n");
		return 1;
	}
	if(!init_ps2_kbd()){
		kerr("Failed to initialize PS2 Keyboard\n");
	}
	if(!init_ps2_mouse()){
		kerr("Failed to initialize PS2 Mouse\n");
	}
	return 0;
}
uint8_t unload(){
	return 0;
}

KERNEL_MODULE("ps2",load,unload,0,"");
