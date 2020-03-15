/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdint.h>

void _start(){
	const char* str  = "SYSCALL 0";
	const char* str1 = "SYSCALL 1";
	
	asm volatile("mov $0x0, %eax");
	asm volatile("mov %0, %%ebx" :: "r"(0x804A000) : "%ebx", "%eax");
	asm volatile("int $0x7F");
	asm volatile("mov $0x0, %eax");
	asm volatile("mov %0, %%ebx" :: "r"(0x804A00A) : "%ebx", "%eax");
	asm volatile("int $0x7F");
	while(1);
}
