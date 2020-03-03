/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once

#include <stdint.h>
#include <stddef.h>

#include <cheader.h>

#define CLOCKS_PER_SEC 1000000

typedef uint32_t clock_t;
typedef uint32_t time_t;

struct tm{
	uint8_t tm_sec;   
	uint8_t tm_min;   
	uint8_t tm_hour; 
	uint8_t tm_mday;  
	uint8_t tm_mon;   
	int32_t tm_year;  //since 1900
	uint8_t tm_wday;  
	uint16_t tm_yday;  
	int8_t tm_isdst; // summer time flag
};

CH_START

char * asctime( const struct tm * tmptr );
char * ctime( const time_t * timeptr );
struct tm * gmtime( const time_t * timeptr );
struct tm * localtime( const time_t * timeptr );
size_t strftime( char * stringptr, size_t maxnum, const char * strformat, const struct tm * timeptr );
	 
clock_t clock( void );
double difftime( time_t timeend, time_t timestart );
time_t mktime( struct tm * ptrtime );
time_t time( time_t * timeptr );

CH_END
