#include <kernel/module/module.h>
#include <kernel/misc/log.h>

char** deps;

uint32_t __exported(){
	kinfo("__exported() kernel call\n");
	return 0xAABBCCD;
}

void load(){
	kinfo("Test module load()\n");
	symbol_export("__exported",&__exported);
}
void unload(){
	
}

KERNEL_MODULE("test",load,unload,0,"");
