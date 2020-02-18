#include <kernel/module/module.h>
#include <kernel/misc/log.h>

uint8_t load(){
	return 0;
}
uint8_t unload(){
	return 0;
}

KERNEL_MODULE("fat",load,unload,1,"ata");
