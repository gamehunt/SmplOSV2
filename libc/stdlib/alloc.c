#include <stdlib.h>

void* malloc(size_t size){
	#ifdef __smplos_libk
		return (void*)kmalloc(size);
	#endif
	return 0;
}

void* valloc(size_t size){
	#ifdef __smplos_libk
		return (void*)kvalloc(size);
	#endif
	return 0;
}

void* realloc(size_t size){
	#ifdef __smplos_libk
		return (void*)krealloc(size);
	#endif
	return 0;
}

void free(void* mem){
	#ifdef __smplos_libk
		return (void*)kfree(mem);
	#endif
}

void vfree(void* mem){
	#ifdef __smplos_libk
		return (void*)kvfree(mem);
	#endif
}
