/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once
#include <stdint.h>

struct idt_entry {
  uint16_t base_low;
  uint16_t sel;
  uint8_t zero;
  uint8_t flags;
  uint16_t base_high;
};

struct idt_ptr {
  uint16_t pad;
  uint16_t size;
  uint32_t base;
};

void idt_set_gate(unsigned char num,unsigned long base,unsigned short sel,unsigned char flags);
void idt_install();
extern void idt_load();

