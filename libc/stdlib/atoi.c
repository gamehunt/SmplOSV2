/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdlib.h>
#include <ctype.h>

int atoi(const char *c)
{
    int value = 0;
    int sign = 1;
    if( *c == '+' || *c == '-' )
    {
        if( *c == '-' ) sign = -1;
        c++;
    }
    while (isdigit(*c))
    {
        value *= 10;
        value += (int) (*c-'0');
        c++;
    }
    return (value * sign);
}
