
#include <kernel/module/module.h>
#include <kernel/module/elf.h>
#include <kernel/fs/vfs.h>
#include <kernel/misc/function.h>

static multiboot_module_t* ramdisk_modules[MODULE_ARRAY_SIZE/2];
static uint32_t last_rmdsk = 0;

static fs_node_t* common_modules[MODULE_ARRAY_SIZE/2];
static uint32_t last_common = 0;

static kernel_module_t* loaded_modules[MODULE_ARRAY_SIZE];
static uint32_t last_loaded = 0;

static char* canonize_absolute(char* path){
	//printf("CANONIZE\n");
	char* npath = kmalloc(sizeof(char)*256);
	uint32_t size = strlen(path);	
	if(path[0]!=','){
		size++;
	}
	if(path[strlen(path)-1] != ','){
		size++;
	}
	npath = kmalloc(size+2);
	memset(npath,0,size+2);
	if(path[0]!=','){
		npath[0]=',';
	}
	strcat(npath,path);
	if(path[strlen(path)-1] != ','){
		strcat(npath,",");
	}
	//printf("CANONIZE END\n");
	return npath; //npath
}

static uint32_t deps_size(char* path){
	uint32_t blocks = 0 ;
	for(int i=0;i<strlen(path);i++){
		if(path[i] == ','){
			blocks++;
		}
	}
	return blocks-1;
}

static char* deps_block(char* path,uint32_t block){
	//printf("BLOCK\n");
	uint32_t blocks = 0 ;
	int a=-1;
	int b=-1;
	for(int i=0;i<strlen(path);i++){
		if(blocks==block && path[i] == ','){
			if(a < 0){
				a = i+1;
			}else{
				b = i;
				break;
			}
		}else if(path[i] == ','){
			blocks++;
		}
	}
	if(a > 0 && b > 0){
		//printf("BLOCK END\n");
		return substr(path,a,b);
	}
	//printf("BLOCK END\n");
	return path;
}

multiboot_module_t* module_ramdisk_get(uint32_t id){
	return ramdisk_modules[id];
}
void module_ramdisk_add(multiboot_module_t* mod){
	ramdisk_modules[last_rmdsk] = mod;
	last_rmdsk++;
}
uint32_t module_ramdisk_count(){
	return last_rmdsk;
}

uint32_t module_common_count(){
	return last_common;
}

uint8_t module_load(fs_node_t* node){
	uint8_t* buffer = kmalloc(node->size);
	if(!knread(node,0,node->size,buffer)){
		kerr("Failed to read module file\n");
	}
	if(elf_load_file(buffer)){
		elf32_hdr_t *ehdr = (elf32_hdr_t *)buffer;
		elf32_sym_t* module_header = (elf32_sym_t*)elf_get_symbol(ehdr,"__module_header");
		if(!module_header){
			kerr("Failed to find module header\n");
			return 0;
		}else{
			elf32_sect_hdr_t *target = elf_section(ehdr, module_header->st_shndx);
			int abs = (int)ehdr + module_header->st_value + target->sh_offset;
			
			kernel_mod_hdr_t* str = (kernel_mod_hdr_t*)abs;
			kernel_module_t* mod_desc = kmalloc(sizeof(kernel_module_t));
			mod_desc->hdr = str;
			memcpy(mod_desc->name,mod_desc->hdr->name,strlen(mod_desc->hdr->name));
			loaded_modules[last_loaded] = mod_desc;
			last_loaded++;
			if(last_loaded+1>=MODULE_ARRAY_SIZE){
				kwarn("Module array is full\n");
			}
			if(!module_check_dependencies(str)){
				mod_desc->state = 2;
				kinfo("Delaying module '%s' due to dependencies\n",str->name);
				return 0;
			}
			kinfo("Loading module '%s'\n",str->name);
			if(str->load()){
				mod_desc->state = 1;
				kinfo("Success!\n");
				module_try_dependencies_reload();
				return 1;
			}else{
				mod_desc->state = 0;
				kerr("Failed!\n");
				return 0;
			}

		}
	}else{
		return 0;
	}
}

void modules_load(){
	kinfo("Loading modules...\n");
	ramdisk_load();
	for(uint32_t i=0;i<module_common_count();i++){
		module_load(common_modules[i]);
	}
	kinfo("Modules loaded\n");
}

uint8_t module_is_loaded(char name[32]){
	for(uint32_t m = 0;m<last_loaded;m++){
		if(!strcmp(name,loaded_modules[m]->name)){
			return 1;
		}
	}
	return 0;
}

uint8_t module_check_dependencies(kernel_mod_hdr_t* kmod){
	//kinfo("HERE\n");
	for(uint8_t m = 0;m<kmod->dep_cnt;m++){
		//kinfo("%s\n",deps_block(kmod->dependencies,m));
		if(!module_is_loaded(deps_block(kmod->dependencies,m))){
			return 0;
		}
	}
	return 1;
}

uint8_t module_try_dependencies_reload(){
	uint8_t flag = 0;
	for(uint8_t m = 0;m<last_loaded;m++){
		//kinfo("%a\n",loaded_modules[m]);
		if(loaded_modules[m]->state == 2){
			
			if(module_check_dependencies(loaded_modules[m]->hdr)){

				if(loaded_modules[m]->hdr->load()){
					loaded_modules[m]->state = 1;
					flag = 1;
				}
			}
		}
	}
	if(flag){
		module_try_dependencies_reload();
	}
}
