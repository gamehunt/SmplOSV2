#include <stdlib.h>

char** environ;
static uint32_t env_size;

static char* env_get_name(char* entry){
	for(uint32_t i=0;i<strlen(entry);i++){
		if(entry[i] == '='){
			return substr(entry,0,i);
		}
	}
	return 0;
}

static char* env_get_value(char* entry){
	for(uint32_t i=0;i<strlen(entry);i++){
		if(entry[i] == '='){
			return substr(entry,i,strlen(entry));
		}
	}
	return 0;
}

static char* env_set_value(char* entry,char* value){
	char* name = env_get_name(entry);
	name = realloc(name,strlen(name)+strlen(value)+2);
	int l = strlen(name);
	name[l] = '=';
	name[l+1] = '\0';
	name = strcat(name,value);
	return name;
}

char* getenv(const char* a){
	if(!env_size){
		return 0;
	}
	for(int i=0;i<env_size;i++){
		if(!strcmp(a,env_get_name(environ[i]))){
			return env_get_value(environ[i]);
		}
	}
	return 0;
}
int setenv(const char *name, const char *value, int overwrite){
	if(!env_size){
		return -1;
	}
	int zero = -1;
	for(int i=0;i<env_size;i++){
		if(!environ[i]){
			zero = i;
			continue;
		}
		if(!strcmp(name,env_get_name(environ[i]))){
			if(overwrite){
				env_set_value(environ[i],value);
			}
			return 0;
		}
	}
	
	if(!env_size){
		environ = malloc(sizeof(char*));
		env_size = 1;
	}else if(zero <= 0){
		env_size++;
		environ = realloc(environ,env_size*sizeof(char*));
	}
	int ind = zero>=0?zero:env_size-1;
	environ[ind] = malloc(strlen(name)+strlen(value)+2);
	strcpy(environ[ind],name);
	environ[ind] = strcat(environ[ind],"=");
	environ[ind] = strcat(environ[ind],value);
	return -1;
}
int unsetenv(const char *name){
	if(!env_size){
		return -1;
	}
	for(int i=0;i<env_size;i++){
		if(!strcmp(name,env_get_name(environ[i]))){
			environ[i] = 0;
		}
	}
	return -1;
}
