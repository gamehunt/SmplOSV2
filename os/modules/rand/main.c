/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <kernel/module/module.h>
#include <kernel/fs/vfs.h>	


extern int is_rdrand_available();
extern uint32_t get_random();

static unsigned long int next = 0;

uint16_t get_pseudorandom(){
	next = next * 1103515245 + 12345;
    return (unsigned int)(next / 65536) % 32768;
}

static uint8_t truerand;

uint32_t rand_read(fs_node_t* node, uint64_t offs, uint32_t size, uint8_t* buffer){
		uint8_t bytes[4];
		for(uint32_t i=0;i<size;i++){
			if(i%4){
				if(truerand){
					uint32_t n = get_random();
					bytes[0] = (n >> 24) & 0xFF;
					bytes[1] = (n >> 16) & 0xFF;
					bytes[2] = (n >> 8) & 0xFF;
					bytes[3] = n & 0xFF;
				}else{
					uint16_t n1 = get_pseudorandom();
					uint16_t n2 = get_pseudorandom();
					bytes[0] = (n1 >> 8) & 0xFF;
					bytes[1] = n1 & 0xFF;
					bytes[2] = (n2 >> 8) & 0xFF;
					bytes[3] = n2 & 0xFF;
				}
			}
			buffer[i] = bytes[i%4];
		}
	return size;
}

	
uint8_t load(){
	truerand = 1;
	if(!is_rdrand_available()){
		kwarn("RDRAND instruction not available! Using pseudorandom generation\n");
		truerand = 0;
		next = rtc_current_time();
	}
	fs_t* randfs = kmalloc(sizeof(fs_t));
	memset(randfs,0,sizeof(fs_t));
	randfs->read = rand_read;
	randfs->name = "random";
	uint32_t idx = register_fs(randfs);
	kmount("/dev/random","",idx);
	
	return 0;
}
uint8_t unload(){
	return 0;
}

KERNEL_MODULE("rand",load,unload,0,"");
