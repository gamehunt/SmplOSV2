/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

void _start(){
	//asm("push 0");
	static int a = 0;
	a++;
	asm("mov %0, %%eax" :: "r"(a) : "%eax");
	asm("int $0x7F");
	//int a = 0/0;
	while(1);
}
