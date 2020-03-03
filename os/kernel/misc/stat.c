/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/misc/stat.h>
#include <kernel/memory/memory.h>

static stat_t* stats[128];
static uint32_t last_stat = 0;

void i_reset_stat(uint32_t idx,uint32_t val){
	if(idx < last_stat){
		stats[idx]->value = val;
	}
}
void reset_stat(char name[32],uint32_t val){
	for(uint16_t i = 0;i<128;i++){
		if(!strcmp(name,stats[i]->name)){
			stats[i]->value = val;
		}
	}
}

uint32_t get_stat(char name[32]){
	for(uint16_t i = 0;i<128;i++){
		//printf("%s %s\n",name,stats[i]->name);
		if(!strcmp(name,stats[i]->name)){
			return stats[i]->value;
		}
	}
	return 0;
}
uint32_t i_get_stat(uint32_t idx){
	if(idx < last_stat){
		return stats[idx]->value;
	}
	return 0;
}
uint32_t create_stat(char name[32],uint32_t init){
	if(!has_stat(name) && last_stat < 128){
		stats[last_stat] = kmalloc(sizeof(stat_t));
		memset(stats[last_stat],0,sizeof(stat_t));
		memcpy(stats[last_stat],name,strlen(name));
		//kinfo("%s %s\n",stats[last_stat],name);
		stats[last_stat]->value = init;
		last_stat++;
		return last_stat-1;
	}
	return 0;
}
void update_stat(char name[32],int32_t update){
	for(uint16_t i = 0;i<128;i++){
		
		if(!strcmp(name,stats[i]->name)){
			stats[i]->value+=update;
			break;
		}
	}
}
void i_update_stat(uint32_t idx,int32_t update){
	if(idx < last_stat){
		stats[idx]->value += update;
	}
}
uint8_t has_stat(char name[32]){
	for(uint16_t i = 0;i<128;i++){
		if(!strcmp(name,stats[i]->name)){
			return 1;
		}
	}
	return 0;
}

