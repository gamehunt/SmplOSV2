/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once

#include <stdint.h>

#include <kernel/fs/vfs.h>

#define TTY_ENABLE 1
#define TTY_DISABLE 0

void init_tty();
	
void tty_set_state(uint8_t);
uint8_t tty_get_state();
