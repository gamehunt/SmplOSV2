#include <kernel/dev/vga.h>
#include <kernel/dev/serial.h>


void eld_init();
void eld_putchar(char c);
void eld_puts(const char* s,int len);

