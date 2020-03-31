#include <stdio.h>
#include <dirent.h>
#include <sys/syscall.h>
//TODO errno? 

DIR * opendir (const char * dirname){
	FILE* f = fopen(dirname,"");
	if(!f){
		return 0;
	}
	DIR* d = malloc(sizeof(DIR));
	d->fd = f->fd;
	free(f);
	d->c_entry = 0;
	return d;
}
int closedir (DIR * dir){
	if(dir && dir->fd != -1){
		sys_close(dir->fd);
		free(dir);
		return 0;
	}
	return -1; 
}
struct dirent * readdir (DIR * dirp){
	struct dirent* dent = malloc(sizeof(struct dirent));
	uint32_t resp = sys_readdir(dirp->fd,dirp->c_entry,dent);
	if(resp){
		free(dent);
		return 0;
	}
	dirp->c_entry++;
	return dent;
}
