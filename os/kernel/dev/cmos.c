#include <kernel/dev/cmos.h>
#include <kernel/io/io.h>

uint8_t cmos_read_value(uint8_t reg){
	asm("cli");
	outb(CMOS_REG,reg|0x80);
	uint8_t val = inb(CMOS_DATA);
	asm("sti");
	return val;
}

void cmos_write_value(uint8_t reg,uint8_t value){
	asm("cli");
	outb(CMOS_REG,reg|0x80);
	outb(CMOS_DATA,value);
	asm("sti");
}
