/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdio.h>
#ifdef __smplos_libk
#include <kernel/fs/vfs.h>
#endif

void puts(const char* str){
	#ifdef __smplos_libk
	if(tty_is_enabled()){
		knwrite(tty_get_root(),0,strlen(str),str);
	}else{
		for(int i=0;i<strlen(str);i++){
			putchar(str[i]);
		}
	}
	#endif
}
