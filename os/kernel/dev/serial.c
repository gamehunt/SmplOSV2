#include <kernel/dev/serial.h>

uint8_t serial_available(uint16_t port){
	return inb(port + 5) & 1;
}
char serial_read(uint16_t port){
	while(!serial_available(port));
	return inb(port);
}

uint8_t serial_can_transmit(uint16_t port){
	return inb(port + 5) & 0x20;
}
void serial_write(uint16_t port, uint8_t data){
	 while (!serial_can_transmit(port));
 
	 outb(port,data);
}
