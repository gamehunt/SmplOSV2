/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/
#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <cheader.h>

#define O_WRONLY 1
#define O_TRUNC  2


CH_START

#if !defined(__smplos_libk) && !defined(__smplos_kernel)
int execv(const char*, char* const[]);
int execve(const char*, char* const[], char* const[]);
int execvp(const char*, char* const[]);
pid_t fork(void);
char* getcwd(char*,int);
int   chdir(const char*);
pid_t getpid();
pid_t getppid(void);  
uid_t getuid();
int   setuid(uid_t uid);
int ioctl(int device, int cmd, void *dx, void *cx);
int read(int fd, void *buf, unsigned count);
int write(int handle, const void *buf, int count);
int open(const char *path, int oflag, ... );
int close(int fd);
long lseek(int handle, long offset, int origin);
long tell(int fd);
int chmod(const char *filename, int mode);
int mkdir(const char *path);
int stat(int fd,stat_t* stat);
int getpriority(int which, int who);
int setpriority(int which, int who, int prio);
#endif

CH_END

