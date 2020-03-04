/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/module/module.h>
#include <kernel/misc/log.h>

uint8_t load(){
	kinfo("Test2 module load()\n");
	return 0;
}
uint8_t unload(){
	return 0;
}

KERNEL_MODULE("test2",load,unload,0,"");
