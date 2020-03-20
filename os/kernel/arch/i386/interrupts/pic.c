/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include<kernel/interrupts/pic.h>


void PIC1_mask_irq(unsigned irq_mask) {
  outb(PIC_A_DATA,inb(PIC_A_DATA) | irq_mask);
}

void PIC2_mask_irq(unsigned irq_mask) {
  outb(PIC_B_DATA,inb(PIC_B_DATA) | irq_mask);
}

void remap_PIC(char offset, char offset1) {
  uint8_t mask;
  uint8_t mask1;

  mask = inb(PIC_A_DATA);
  mask1 = inb(PIC_B_DATA);

  outb(PIC_A, 0x11);          // init in cascade mode
  outb(PIC_B, 0x11);

  outb(PIC_A_DATA, offset);
  outb(PIC_A_DATA, offset1);  // set offset for the 2 PICs

  outb(PIC_A_DATA,4);                    // tells the master PIC there is a slave PIC at IRQ 2 (0000 0100)
  outb(PIC_B_DATA, 2);        // tells the slave PIC its on the second IRQ (0000 0010)

  outb(PIC_A_DATA, 0x01);
  outb(PIC_B_DATA, 0x01);

  outb(PIC_A_DATA, mask);
  outb(PIC_B_DATA, mask1);

  PIC1_mask_irq(0xFC);
}

