/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once

#include <stdint.h>

#include <kernel/fs/vfs.h>

#define TTY_OUTPUT_STREAM_STDOUT 0
#define TTY_OUTPUT_STREAM_STDERR 1

void init_tty();
	
uint8_t tty_is_enabled();


fs_node_t* tty_get_root();

void tty_set_output_stream();
uint32_t tty_get_output_stream();
