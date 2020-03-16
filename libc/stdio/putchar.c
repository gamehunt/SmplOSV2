/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdio.h>
#ifdef __smplos_libk
	#include<kernel/io/terminal.h>
	#include<kernel/fs/vfs.h>
	static fs_node_t* stream = 0;
	static uint8_t id = 100;
	void set_output_stream(uint8_t str){
		if(!tty_get_state()){
			return;
		}
		if(str != id){
			stream = kseek(str?"/dev/stderr":"/dev/stdout");
			id = str;
		}
	}
	fs_node_t* get_output_stream(){
		return stream;
	}
	uint8_t get_output_stream_id(){
		return id;
	}
#endif
void putchar(char c){
	#ifdef __smplos_libk
	if(tty_get_state() && stream){
		knwrite(stream,0,1,&c);
	}else{
		terminal_putchar(c);
	}
	#endif
}
