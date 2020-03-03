/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/


#pragma once

#define NULL 0

struct registers{
	unsigned int gs,fs,es,ds;
	unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
	unsigned int int_no, err_code;
	unsigned int eip, cs, eflags, useresp, ss;  
};
typedef struct registers * regs_t;

typedef struct{
	regs_t regs;
	const char* description;
	const char* extra_info;
}crash_info_t;
