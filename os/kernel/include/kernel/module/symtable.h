/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once

#include <stdint.h>
#include <kernel/misc/function.h>

struct symtable_entry{
	uint32_t* addr;
	char name[64];
};
typedef struct symtable_entry sym_entry_t;

sym_entry_t* symbol_export(char name[64],uint32_t* addr);
sym_entry_t* symbol_get(uint32_t id);
sym_entry_t* symbol_seek(char name[64]);

void init_symtable();


