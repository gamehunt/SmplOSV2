/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/
#pragma once
#if !defined(__smplos_libk) && !defined(__smplos_kernel)
#include <sys/types.h>
int execv(const char*, char* const[]);
int execve(const char*, char* const[], char* const[]);
int execvp(const char*, char* const[]);
pid_t fork(void);
char* getcwd(char*,int);
int   chdir(char*);
pid_t getpid();
pid_t getppid(void);  
uid_t getuid();
int   setuid(uid_t uid);
#endif
