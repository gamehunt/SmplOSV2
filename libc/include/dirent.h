#pragma once

#include <stdint.h>
#include <cheader.h>

CH_START

struct dirent{
	char d_name[256];
	int d_ino;
};

typedef struct{
	int fd;
	int c_entry;
}DIR;

DIR * opendir (const char * dirname);
int closedir (DIR * dir);
struct dirent * readdir (DIR * dirp);

CH_END
