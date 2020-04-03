/*	

    Copyright (C) 2020
     
    Author: gamehunt 

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <kernel/interrupts/syscalls.h>
#include <sys/syscall.h>
#include <kbd.h>

char cwdbuffer[256];

char*       exec = 0;
char**      argv = 0;
uint32_t    argc = 0;

FILE* exec_stdout = 0;
FILE* exec_stderr = 0;
FILE* exec_stdin  = 0;


static volatile uint8_t in_exec = 0;
static uint8_t login_only = 0; //Launch login and exit
static uint8_t standalone_mode = 0;//Standalone mode(without compositor)

int sig_child(){
	if(login_only){
		exit(0);
	}
	char proc_out[256];
	memset(proc_out,0,256);
	if(exec_stdout && fread(proc_out,1,256,exec_stdout)){
		printf("%s",proc_out);
	}
	if(exec_stdout){
		fclose(exec_stdout);
		exec_stdout = 0;
	}
	if(exec_stderr){
		fread(proc_out,1,128,exec_stderr);
		fclose(exec_stderr);
		exec_stderr = 0;
	}
	if(exec_stdin){
		fread(proc_out,1,256,exec_stdin);
		fclose(exec_stdin);
		exec_stdin = 0;
	}
	in_exec = 0;
	printf("\n[process exited]\n");
	printf("[%s %d]>> ",getcwd(cwdbuffer,256)?cwdbuffer:"ERROR",getuid());
	sys_sigexit();
}


void process_word(uint8_t* word){
	if(!exec){
		exec = malloc(strlen(word)+1);
		memset(exec,0,strlen(word)+1);
		strcpy(exec,word);
	}else{
		if(!argc){
			argv = malloc(sizeof(char*));
		}else{
			argv = realloc(argv,sizeof(char*)*(argc+1));
		}
		argv[argc] = malloc(strlen(word)+1);
		memset(argv[argc],0,strlen(word)+1);
		strcpy(argv[argc],word);
		argc++;
	}
	//printf("Processed\n");
}


char* seekenv(char* cmd){
	char* path = getenv("PATH");
	char* part = strtok(path,":");
	char* fullname = malloc(strlen(cmd)+5);
	memset(fullname,0,strlen(cmd)+5);
	while(part){
		strcpy(fullname,cmd);
		strcat(fullname,".smp");
		char* fullpath = malloc(256);
		memset(fullpath,0,256);
		strcpy(fullpath,part);
		strcat(fullpath,"/");
		strcat(fullpath,fullname);
		FILE* f;
		if(f = fopen(fullpath,"r")){
			fclose(f);
			free(fullname);
			free(part);
			return fullpath;
		}
		part = strtok(NULL,":");
	}
	return 0;
}

void process_input(uint8_t* buffer,uint32_t buff_size){
	char* word = strtok(buffer," ");
	while(word){
		process_word(word);
		word = strtok(NULL," ");
	}
	if(exec){
		if(argc){
			for(int i=0;i<argc;i++){
				if(argv[i][0] == '$'){
					char* newarg = getenv(strtok(argv[i],"$"));
					if(!newarg){
						newarg = " ";
					}
					free(argv[i]);
					argv[i] = malloc(strlen(newarg)+1);
					strcpy(argv[i],newarg);
				}
			}
		}
		if(!strcmp("cd",exec)){ //TODO make proper system for builtin shell cmds
			if(argc){
				if(chdir(argv[0]) < 0){
					printf("No such directory\n");
				}
			}else{
				printf("Usage: cd [path]\n");
			}
		}else{
			char* fullpath = 0;
			if(exec[0] == '.'){
				FILE* f;
				fullpath = strcat(getcwd(cwdbuffer,256),exec);
				if(!(f = fopen(fullpath,"r"))){
					fullpath = 0;
				}else{
					fclose(f);
				}
			}else if(exec[0] == '/'){
				fullpath = exec;
			}else{
				fullpath = seekenv(exec);
			}
			if(fullpath){
				uint32_t pid = execv(fullpath,argv);
				if(!pid){
					printf("Failed to execute: %s\n",buffer);
				//return;
				}else{
					//TODO: optimize
					//printf("In exec\n");
					in_exec = 1;
					char path[64];
					memset(path,0,64);
					sprintf(path,"/proc/%d/stdout",pid);
					while(!(exec_stdout = fopen(path,"r")));
					memset(path,0,64);
					sprintf(path,"/proc/%d/stderr",pid);
					while(!(exec_stderr = fopen(path,"r")));
					memset(path,0,64);
					sprintf(path,"/proc/%d/stdin",pid);
					while(!(exec_stdin = fopen(path,"w")));
				}
			}else{
				printf("Executable not found: %s\n",exec);
			}
		}
		//printf("Freeing %d arguments from addr %a\n",argc,argv);
		for(uint32_t i = 0;i<argc;i++){
			free(argv[i]);
		}
		if(argv){
			free(argv);
		}
		free(exec);
		exec = 0;
		argv = 0;
		argc = 0;

	}
}

static void process_args(int argc,char** argv){
	if(!argc){
		return;
	}
	for(int i=0;i<argc;i++){
		if(argv[i][0] == '-'){
			for(int j=1;j<strlen(argv[i]);j++){
				switch(argv[i][j]){
					case 'L':
						login_only = 1;
					break;
					case 's':
						standalone_mode = 1;
					break;
				}
			}
		}
	}
}

int main(int argc,char** argv,char** envp){
	

	process_args(argc,argv);
	
	if(standalone_mode){
		
		FILE* f_stdout = malloc(sizeof(FILE));
		f_stdout->fd = 0;
		fclose(f_stdout);
		if(!fopen("/dev/tty","w")){
			return 1;
		}

		key_t* key = malloc(sizeof(key_t));
		uint8_t* pipe_buffer = malloc(128);
		uint8_t* cmd_buffer  = malloc(2048);
		uint16_t cmd_buff_idx = 0;
		
		FILE* kbd = fopen("/dev/kbd","r");
	
		sys_signal(SIG_CHILD,sig_child);
	
		if(login_only){
			process_input("/usr/bin/login.smp",strlen("/usr/bin/login.smp"));
		}else{
			if(setenv("PATH","/usr/bin",1) < 0){
				printf("Failed to create environment!\n");
				return 1;
			}
			printf("Launched standalone shell\n");
			uint32_t random = 0;
			FILE* random_file = fopen("/dev/random","r");
			fread(&random,4,1,random_file);
			fclose(random_file);
			printf("\nToday's random integer is %a\n\n",random);
		}
	
		while(1){
			uint32_t nodes[2];
			nodes[0] = 0;
			nodes[1] = 0;
			uint32_t cnt = 0;
			if(exec_stdout){
				nodes[0] = exec_stdout->fd;
				nodes[1] = kbd->fd;
				cnt = 2;
			}else{
				nodes[0] = kbd->fd;
				cnt = 1;
			}
			sys_fswait(nodes,cnt); //TODO return which node awaked us
			char proc_stdout[256];
			memset(proc_stdout,0,256);
			if(in_exec && exec_stdout && fread(proc_stdout,1,256,exec_stdout)){
				printf("%s",proc_stdout);
			}
			memset(key,0,sizeof(key_t));
			memset(pipe_buffer,0,128);
			uint32_t read = fread(pipe_buffer,1,128,kbd);
			for(uint32_t i=0;i<read;i++){
				kbd_key_event(key,pipe_buffer[i]);
				if(key->key && key->state){
					if(key->key == 0x8 && cmd_buff_idx){
						cmd_buff_idx--;
						cmd_buffer[cmd_buff_idx] = 0;
						putchar(0x8);
						continue;
					}else if(key->key == 0x8){
						continue;
					}
					if(key->key == '\n'){
						//printf("in_exec: %d\n",in_exec);
						if(cmd_buff_idx){
							putchar('\n');
							cmd_buffer[cmd_buff_idx+1] = '\0';
							cmd_buff_idx++;
							if(!in_exec){
								process_input(cmd_buffer,cmd_buff_idx); //Process shell cmd
							}else{
								fwrite(cmd_buffer,cmd_buff_idx,1,exec_stdin); //Send to process
							}
							memset(cmd_buffer,0,cmd_buff_idx);
							cmd_buff_idx=0;
						}
						if(!in_exec){
							printf("\n[%s %d]>> ",getcwd(cwdbuffer,256)?cwdbuffer:"ERROR",getuid());
						}
					}else{
						putchar(key->key);
						cmd_buffer[cmd_buff_idx] = key->key;
						cmd_buff_idx++;
					}
				}
			}
		}
	}else{
		//TODO compositor 
		
	}
}
