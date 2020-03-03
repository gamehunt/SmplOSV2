/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/module/module.h>
#include <kernel/misc/log.h>

char** deps;

uint32_t __exported(){
	kinfo("__exported() kernel call\n");
	return 0xAABBCCD;
}

uint8_t load(){
	kinfo("Test module load()\n");
	symbol_export("__exported",&__exported);
	return 0;
}
uint8_t unload(){
	return 0;
}

KERNEL_MODULE("test",load,unload,0,"");
