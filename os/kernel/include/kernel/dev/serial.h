#pragma once

#include <kernel/io/io.h>

#define SERIAL_COM1 0x3F8
#define SERIAL_COM2 0x2F8
#define SERIAL_COM3 0x3E8
#define SERIAL_COM4 0x2E8

uint8_t serial_available(uint16_t port);
char serial_read(uint16_t port);

uint8_t serial_can_transmit(uint16_t port);
void serial_write(uint16_t port, uint8_t data);
