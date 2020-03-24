/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/
#pragma once
#if !defined(__smplos_libk) && !defined(__smplos_kernel)
int execv(const char*, char* const[]);
int execve(const char*, char* const[], char* const[]);
int execvp(const char*, char* const[]);
pid_t fork(void);
#endif
