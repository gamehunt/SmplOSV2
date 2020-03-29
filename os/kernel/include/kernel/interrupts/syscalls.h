/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdint.h>

#define SYS_ECHO     0
#define SYS_READ     1
#define SYS_WRITE    2
#define SYS_OPEN     3
#define SYS_CLOSE    4
#define SYS_YIELD    5 
#define SYS_EXEC     6
#define SYS_CLONE    7 //currently dont work
#define SYS_IOCTL    8
#define SYS_READDIR  9
#define SYS_FSWAIT   10
#define SYS_EXIT     11
#define SYS_SBRK     12
#define SYS_ASSIGN   13
#define SYS_SIG      14
#define SYS_SIGHANDL 15
#define SYS_SIGEXIT  16
#define SYS_TIME     17
#define SYS_WAITPID  18

void init_syscalls();
