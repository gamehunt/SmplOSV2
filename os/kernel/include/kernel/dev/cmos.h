#pragma once

#include <stdint.h>

#define CMOS_REG 0x70
#define CMOS_DATA 0x71



uint8_t cmos_read_value(uint8_t reg);
void cmos_write_value(uint8_t reg,uint8_t value);
