/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdlib.h>

void* malloc(size_t size){
	#ifdef __smplos_libk
		return (void*)kmalloc(size);
	#endif
	return 0;
}

void* valloc(size_t size,uint32_t alig){
	#ifdef __smplos_libk
		return (void*)kvalloc(size,alig);
	#endif
	return 0;
}

void* realloc(uint32_t* ptr,size_t size){
	#ifdef __smplos_libk
		return (void*)krealloc(ptr,size);
	#endif
	return 0;
}

void free(void* mem){
	#ifdef __smplos_libk
		return (void*)kfree(mem);
	#endif
}
