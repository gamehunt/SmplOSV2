/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once

#include <stdint.h>
#include <stddef.h>

#include <cheader.h>

CH_START

int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);

int strlen(const char* str);
char * strcat(char *dest, const char *src);
uint32_t strcmp(const char *X, const char *Y);
char* substr(const char *src, int m, int n);
void strcpy(char dest[], const char source[]);
char* strtok(char* str,const char* delimiter);
const char* strchr(const char* str,int sym);
CH_END
