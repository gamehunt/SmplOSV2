//#include <stdint.h>

//void dump_vfs();

void kinfo(const char* format,...);

struct kernel_mod_hdr{
	char* name;
	int(*load)();
	int(*unload)();
};
typedef struct kernel_mod_hdr kernel_mod_hdr_t;

int main(){
	kinfo("TEST\n");

	return 0;
}

kernel_mod_hdr_t __module_header = {"test",&main,&main};


