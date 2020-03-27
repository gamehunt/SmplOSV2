/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <cheader.h>

CH_START

#if !defined(__smplos_libk) && !defined(__smplos_kernel)
typedef struct{
	uint32_t fd;
	//TODO
}FILE;

extern FILE* stderr;
extern FILE* stdout;
extern FILE* stdin;

#define stderr stderr
#define stdout stdout
#define stdin  stdin

#define SEEK_SET 0
#endif

void putchar(char c);

void puts(const char* str);

void vprintf(const char* restrict format,va_list argptr);
void printf(const char* restrict str,...);
void vsprintf(char* buffer, const char* format,va_list argptr);
void sprintf( char *buffer, const char *format, ... );

#if !defined(__smplos_libk) && !defined(__smplos_kernel)

int fflush(FILE *stream);
int fprintf(FILE *fp, const char* restrict format, ...);
int vfprintf(FILE*, const char* restrict f, va_list);
int fclose(FILE*);
FILE* fopen(const char*, const char*);
size_t fread(void*, size_t, size_t, FILE*);
int fseek(FILE*, long, int);
long ftell(FILE*);
size_t fwrite(const void*, size_t, size_t, FILE*);
void setbuf(FILE*, char*);
void fputc(char,FILE*);
void fputs(const char*, FILE*);

#endif
CH_END
