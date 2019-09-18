#pragma once

#include <stddef.h>

#include <cheader.h>

CH_START

__attribute__((__noreturn__)) void abort(void);

void* malloc(size_t size);
void* valloc(size_t size);
void* realloc(size_t size);

void free(void* mem);
void vfree(void* mem);

long long int atoi(const char *c);
CH_END
