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
//#define SYS_CLONE    7 
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
#define SYS_GETCWD   19
#define SYS_CHDIR    20
#define SYS_SETUID   21
#define SYS_GETUID   22
#define SYS_GETPID   23
#define SYS_LINK      24
#define SYS_SLEEP       25
#define SYS_GETPPID       26
#define SYS_PIPE            27
#define SYS_PWREQ           28
#define SYS_FSSTAT          29
#define SYS_PRIOR           30
#define SYS_THREAD          31
#define SYS_SHMEM_CREATE    32
#define SYS_SHMEM_OPEN      33
#define SYS_SHMEM_RESET     34
//vfs opened node flags. TODO: append and truncate
#define F_READ   	1
#define F_WRITE  	2
#define F_CREATE 	4

void init_syscalls();
