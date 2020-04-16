/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#pragma once
#include <stdint.h>

typedef struct{
	char name[32];
	uint32_t value;
}kstat_t;

void i_reset_stat(uint32_t idx,uint32_t val);
void reset_stat(char name[32],uint32_t val);
uint32_t get_stat(char name[32]);
uint32_t i_get_stat(uint32_t idx);
uint32_t create_stat(char name[32],uint32_t init);
void update_stat(char name[32],int32_t update);
void i_update_stat(uint32_t idx,int32_t update);
uint8_t has_stat(char name[32]);
