#pragma once

typedef uint32_t sig_atomic_t;

#include <cheader.h>

CH_START

typedef void (*signal (int sig, void (*sigfunc) (int func)))(int);
int raise(int sig);

CH_END
