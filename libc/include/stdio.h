/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once
#include <stdarg.h>
#include <stdint.h>

#include <cheader.h>

#ifdef __smplos_libk

#include<kernel/fs/vfs.h>

#define STDOUT 0
#define STDERR 1

#endif

CH_START

void putchar(char c);

void puts(const char* str);

void vprintf(const char* restrict format,va_list argptr);
void printf(const char* restrict str,...);
void vsprintf(char* buffer, const char* format,va_list argptr);
void sprintf( char *buffer, const char *format, ... );

#ifdef __smplos_libk
void set_output_stream(uint8_t stream);
fs_node_t* get_output_stream();
uint8_t get_output_stream_id();
#endif

CH_END
