/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <cheader.h>

CH_START

__attribute__((__noreturn__)) void abort(void);



void* malloc(size_t size);
void* valloc(size_t size,uint32_t alig);
void* realloc(uint32_t* ptr,size_t size);
void* calloc(uint32_t num,size_t size);

void free(void* mem);

int       atoi(const char *c);
double    atof(const char *c);
long      atol(const char *c);
long long atoll(const char *c);

int abs(int j);
long labs(long j);

double strtod(const char *start, char **end);
long strtol(const char *start, char **end, int radix);
unsigned long strtoul(const char *start, char **end, int radix);

typedef struct {
    int quot, rem;
} div_t;

typedef struct {
    long int quot, rem;
} ldiv_t;

div_t div( int numer,  int  denom );
ldiv_t ldiv( long numer, long denom );

void *bsearch(const void *key, const void *base, size_t nitems, size_t size, int (*compar)(const void *, const void *));
void qsort( const void *ptr, size_t count, size_t size,int (*comp)(const void *, const void *) );


#if !defined(__smplos_libk) && !defined(__smplos_kernel)

#define MAX_CXA_HANDLERS 256

typedef void (*atexit_handl_t) (void *);
typedef struct{
	void* arg;
	atexit_handl_t handl;
}cxa_atexit_handler;


int atexit(void (*)(void));
char* getenv(const char*);
int setenv(const char *name, const char *value, int overwrite);
int unsetenv(const char *name);
void exit(int status);

int system(const char *str);

void srand( unsigned seed );
int rand( void );



void fix_user_heap();
#endif
CH_END
