/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once

#include <stdint.h>

#include <cheader.h>

CH_START

uint8_t isdigit(char c); 
uint8_t isxdigit(int c);
uint8_t isprint(int c);
uint8_t isspace(int c);
int tolower(int ch);
int toupper(int ch);
CH_END
