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

void free(void* mem);

long long int atoi(const char *c);
#if !defined(__smplos_libk) && !defined(__smplos_kernel)
int atexit(void (*)(void));
char* getenv(const char*);
int setenv(const char *name, const char *value, int overwrite);
int unsetenv(const char *name);

void fix_user_heap();
#endif
CH_END
