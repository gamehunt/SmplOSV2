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

#define BUFSIZ		1024

typedef struct{
	uint32_t fd;
	char ungetc;
	char* buffer;
	int offset;
	int size;
	int eof;
	int error;
	//TODO
}FILE;

extern FILE* stderr;
extern FILE* stdout;
extern FILE* stdin;

#define stderr stderr
#define stdout stdout
#define stdin  stdin

#define SEEK_SET  0
#define SEEK_CUR  1
#define SEEK_END  2
#define EOF       (-1)

#define putc fputc

#endif

int putchar(char c);

int puts(const char* str);

int  vprintf(const char* format,va_list argptr);
int  printf(const char* str,...);
int  vsprintf(char* buffer, const char* format,va_list argptr);
int  sprintf( char *buffer, const char *format, ... );

#if !defined(__smplos_libk) && !defined(__smplos_kernel)

typedef uint32_t fpos_t;

#define _IOFRF 1
#define _IONBF 2
#define _IOLBF 3

int fflush(FILE *stream);
int fprintf(FILE *fp, const char* format, ...);
int vfprintf(FILE*, const char* f, va_list);
int fclose(FILE*);
FILE* fopen(const char*, const char*);
FILE *fdopen(int fildes, const char *mode);
size_t fread(void*, size_t, size_t, FILE*);
int fseek(FILE*, long, int);
long ftell(FILE*);
size_t fwrite(const void*, size_t, size_t, FILE*);
void setbuf(FILE*, char*);
int fputc(char,FILE*);
int fputs(const char*, FILE*);
char *fgets(char *str, int num, FILE *stream);
int getc(FILE *stream);
void clearerr(FILE *stream);
int feof(FILE *stream);
int ferror(FILE *stream);
int fgetc(FILE *stream);
int fgetpos(FILE *stream, fpos_t *pos);
int fsetpos(FILE *stream, const fpos_t *pos);
FILE *freopen(const char *fname, const char *mode, FILE *stream);
int fscanf(FILE *stream, const char *format, ...);
int getchar(void);
void perror(const char *str);
int remove(const char *fname);
int rename(const char *oldfname, const char *newfname);
void rewind(FILE *stream);
int scanf(const char *format, ...);
int sscanf(const char *buf, const char *format, ...);
int setvbuf(FILE *stream, char *buf, int mode, size_t size);
FILE *tmpfile(void);
int ungetc ( int character, FILE * stream );
int fileno(FILE *stream);
#endif

CH_END
