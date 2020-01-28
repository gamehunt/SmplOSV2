#include <kernel/module/module.h>

void kinfo(const char* format,...);

void load(){
	kinfo("Test module load()\n");
}
void unload(){
	
}

KERNEL_MODULE("test",load,unload);
