/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once

#include<kernel/io/io.h>

#define PIC_A 0x20
#define PIC_B 0xA0
#define PIC_A_DATA 0x21
#define PIC_B_DATA 0xA1
#define PIC_ENDOFINT 0x20

void remap_PIC(char offset,char offset1);
void PIC1_mask_irq(unsigned irq_mask);
void PIC2_mask_irq(unsigned irq_mask);  

