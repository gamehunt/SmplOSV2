/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once

#include <stdio.h>
#include <stdlib.h>

CH_START

#ifndef NDEBUG
	static inline void __assert_fail(){
		printf("Assertion failed\n");
		abort();
	}
	#define assert(exp) (exp?(void)0:__assert_fail())		
#else
	#define assert(exp) ((void)0)
#endif

CH_END
