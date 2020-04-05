/*	

    Copyright (C) 2020
     
    Author: gamehunt 


	Simple interactive shell TODO: launch child process in another way. We should use compositor
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

int sig_child(){
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

int main(int argc,char** argv,char** envp){
	uint8_t* cmd_buffer  = malloc(2048);
	char cwd[256];
	sys_signal(SIG_CHILD,sig_child);
	
	printf("Launched interactive shell session\n");
	printf("[%s %d]>> ",getcwd(cwdbuffer,256)?cwdbuffer:"ERROR",getuid());
	
	while(1){
		
		uint32_t readen = 0;
		char proc_stdout[256];
		memset(proc_stdout,0,256);
		if(exec_stdout){
			readen = fread(proc_stdout,1,256,exec_stdout);
		}
		
		
		for(uint32_t i =0;i<readen;i++){
			putchar(proc_stdout[i]);
		}
				
		readen = fread(cmd_buffer,1,2048,stdin);
		if(readen){				
			if(in_exec){
				fwrite(cmd_buffer,1,1,exec_stdin);
			}else{
				if(cmd_buffer[0] != '\n'){
					putchar('\n');
					cmd_buffer[readen-1] = '\0';
					process_input(cmd_buffer,readen);
				}
				memset(cmd_buffer,0,2048);
				if(!in_exec){
					printf("[%s %d]>> ",getcwd(cwdbuffer,256)?cwdbuffer:"ERROR",getuid());
				}
			}
		}
	}
}
