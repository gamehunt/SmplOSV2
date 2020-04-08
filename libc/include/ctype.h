/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once

#include <stdint.h>

#include <cheader.h>

CH_START

int isdigit(char c); 
int isxdigit(int c);
int isprint(int c);
int isspace(int c);
int isalnum(int ch);
int isalpha(int ch);
int iscntrl(int ch);
int isgraph(int ch);
int islower(int ch);
int ispunct(int ch);
int isupper(int ch);

int tolower(int ch);
int toupper(int ch);

CH_END
