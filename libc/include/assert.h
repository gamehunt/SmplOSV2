/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once

#include <stdio.h>
#include <stdlib.h>

CH_START

#ifndef NDEBUG
	#define __assert_fail(num,name,exp)\
		printf("Assertion failed: line %d in file %s: %s\n",num,name,exp);\
		abort();
	#define assert(exp) (exp?(void)0:__assert_fail( __LINE__, __FILE__, #exp))		
#else
	#define assert(exp) ((void)0)
#endif

CH_END
