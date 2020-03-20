/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once

#define GET_BIT(n,k)   ((n>>k) & 1U)
#define SET_BIT(n,k)   (n | (1UL << k))
#define CLEAR_BIT(n,k) (n & (~(1UL << k)))
#define TOGGL_BIT(n,k) (n ^ (1UL << k))
