#pragma once

typedef uint32_t* jmp_buf;

#include <cheader.h>

CH_START

int setjmp(jmp_buf envbuf);
int longjmp(jmp_buf envbuf);

CH_END
