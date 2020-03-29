#include <dirent.h>

//TODO errno? 

DIR * opendir (const char * dirname){
	uint32_t fd = fopen(dirname,"");
	DIR* d = malloc(sizeof(DIR));
	d->fd = fd;
	d->c_entry = 0;
	return d;
}
int closedir (DIR * dir){
	if(dir && dir->fd != -1){
		fclose(dir->fd);
		free(dir);
		return 0;
	}
	return -1; 
}
struct dirent * readdir (DIR * dirp){
	struct dirent* dent = malloc(sizeof(struct dirent));
	uint32_t resp = sys_readdir(dirp->fd,++dirp->c_entry,dent);
	if(resp){
		memset(dent,0,sizeof(struct dirent));
	}
	return dent;
}
