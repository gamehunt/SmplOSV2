#pragma once 


extern int *__geterrno(void);
#define errno (*__geterrno())

//TODO
