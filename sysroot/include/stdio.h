#pragma once
#include <stdarg.h>
#include <stdint.h>

#include <cheader.h>

CH_START

void putchar(char c);

void puts(const char* str);

void vprintf(const char* restrict format,va_list argptr);
void printf(const char* restrict str,...);
void vsprintf(char* buffer, const char* format,va_list argptr);
void sprintf( char *buffer, const char *format, ... );

CH_END
