#include <kernel/dev/tty.h>
#include <kernel/fs/vfs.h>
#include <kernel/memory/memory.h>

uint16_t tty_fsid;

fs_node_t* tty_root = 0;
uint32_t tty_stream = 0;

fs_node_t* tty_mount(fs_node_t* root){
	//printf("HERE %s\n",root->name);
	return root;
}

uint32_t tty_write(fs_node_t* node, uint64_t offset, uint32_t size, uint8_t* buffer){
	for(uint32_t i=0;i<size;i++){
		terminal_putchar(buffer[i]);
	}
	return size;
}

uint8_t tty_is_enabled(){
	return 0;
}

fs_node_t* tty_get_root(){
	return tty_root;
}

void tty_set_output_stream(uint32_t stream){
	tty_stream = stream;
	if(stream == TTY_OUTPUT_STREAM_STDOUT){
		tty_root = kseek("/dev/stdout/");	
	}else{
		tty_root = kseek("/dev/stderr/");
	}
}

uint32_t tty_get_output_stream(){
	return tty_stream;
}


void init_tty(){
	
	fs_t tty_fs;
	int* dada = kmalloc(sizeof(int));
	*dada = 40;
	tty_fs.mount = &tty_mount;
	tty_fs.write = &tty_write;
		
	tty_fsid = register_fs(&tty_fs);
	if(kmount("/dev/stdout/",tty_fsid)){
		if(!kmount("/dev/stderr/",tty_fsid)){
			kerr("Failed to create stderr output handler\n");
		}
		tty_set_output_stream(TTY_OUTPUT_STREAM_STDOUT);
		kinfo("TTY initialized: %d fsid\n",tty_fsid);
	}else{
		kerr("Failed to initialize tty\n");
	}
}
