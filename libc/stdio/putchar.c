#include <stdio.h>
#ifdef __smplos_libk
	#include<kernel/io/terminal.h>
	#include<kernel/fs/vfs.h>
#endif
void putchar(char c){
	#ifdef __smplos_libk
	if(tty_is_enabled()){
		knwrite(tty_get_root(),0,1,&c);
	}else{
		terminal_putchar(c);
	}
	#endif
}
