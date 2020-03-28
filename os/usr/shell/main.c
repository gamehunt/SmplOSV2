/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kernel/interrupts/syscalls.h>
#include <sys/syscall.h>
#include <kernel/fs/vfs.h>
#include <kbd.h>

void process_word(uint8_t* word,uint32_t wlen){
	if(!strcmp(word,"dbg")){
		printf("echo\n");
	}else{
		printf("Unknown cmd\n");
	}
}

void process_input(uint8_t* buffer,uint32_t buff_size){
	uint8_t* word = malloc(buff_size);
	uint32_t i=0;
	uint32_t w_ptr = 0;
	while(i < buff_size){
			if(buffer[i] == ' '){
				word[w_ptr] = '\0';
				process_word(word,w_ptr+1);
				w_ptr = 0;
				i++;
				continue;
			}
			word[w_ptr] = buffer[i];
			i++;
			w_ptr++;
	}
	if(w_ptr){
		word[w_ptr] = '\0';
		process_word(word,w_ptr+1);
	}
	free(word);
}

int main(){
	//while(1);
	printf("Launched shell\n>> ");
	FILE* kbd = fopen("/dev/kbd","");
	key_t* key = malloc(sizeof(key_t));
	uint8_t* pipe_buffer = malloc(128);
	uint8_t* cmd_buffer  = malloc(2048);
	uint16_t cmd_buff_idx = 0;
	while(1){
		memset(key,0,sizeof(key_t));
		memset(pipe_buffer,0,128);
		sys_fswait(&kbd->fd,1);
		uint32_t read = sys_read(kbd->fd,0,128,pipe_buffer);
		for(uint32_t i=0;i<read;i++){
			kbd_key_event(key,pipe_buffer[i]);
			if(key->key && key->state){
				if(key->key == '\n'){
					if(cmd_buff_idx){
						putchar('\n');
						cmd_buffer[cmd_buff_idx+1] = '\0';
						cmd_buff_idx++;
						process_input(cmd_buffer,cmd_buff_idx);
						memset(cmd_buffer,0,cmd_buff_idx);
						cmd_buff_idx=0;
					}
					printf("\n>> ");
				}else{
					putchar(key->key);
					cmd_buffer[cmd_buff_idx] = key->key;
					cmd_buff_idx++;
				}
			}
		}
	}
}
