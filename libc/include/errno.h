/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once 


extern int *__geterrno(void);
#define errno (*__geterrno())

//TODO
