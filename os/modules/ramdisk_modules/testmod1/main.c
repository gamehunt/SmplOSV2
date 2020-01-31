#include <kernel/module/module.h>
#include <kernel/misc/log.h>


void load(){
	kinfo("Test1 module load()\n");
}
void unload(){
	
}

KERNEL_MODULE("test1",load,unload,1,"test");
