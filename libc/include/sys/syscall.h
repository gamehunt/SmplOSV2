#include <stdint.h>
#include <dirent.h>
#include <sys/types.h>
#include <kernel/interrupts/syscalls.h>
#include <kernel/proc/proc.h>
#include <cheader.h>

CH_START

#if !defined(__smplos_libk) && !defined(__smplos_kernel) 

#define SYS_PWREQ_SHUTDOWN 0
#define SYS_PWREQ_REBOOT   1

extern uint32_t sys_call(uint32_t n,uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e);

static inline uint32_t sys_echo(char* str,uint32_t opt){
	return sys_call(SYS_ECHO,(uint32_t)str,opt,0,0,0);
}

static inline uint32_t sys_read(uint32_t fd,uint64_t offset, uint32_t size,uint8_t* buffer){
	uint32_t offs_high = (uint32_t)(offset >> 32);
	uint32_t offs_low = (uint32_t)(offset);
	return sys_call(SYS_READ,fd,offs_high,offs_low,size,(uint32_t)buffer);
}

static inline uint32_t sys_write(uint32_t fd,uint64_t offset,uint32_t size,uint8_t* buffer){
	uint32_t offs_high = (uint32_t)(offset >> 32);
	uint32_t offs_low = (uint32_t)(offset);
	return sys_call(SYS_WRITE,fd,offs_high,offs_low,size,(uint32_t)buffer);
}

static inline uint32_t sys_open(char* path,uint8_t flags){
	return sys_call(SYS_OPEN,(uint32_t)path,flags,0,0,0);
}


static inline uint32_t sys_close(uint32_t fd){
	return sys_call(SYS_CLOSE,fd,0,0,0,0);
}

static inline uint32_t sys_readdir(uint32_t fd,uint32_t index,struct dirent* ptr){
	return sys_call(SYS_READDIR,fd,index,(uint32_t)ptr,0,0);
}

static inline uint32_t sys_exec(char* path,char** argv,char** envp){
	return sys_call(SYS_EXEC,(uint32_t)path,(uint32_t)argv,(uint32_t)envp,0,0);
}

static inline uint32_t sys_clone(){
	return sys_call(SYS_CLONE,0,0,0,0,0);
}

static inline uint32_t sys_ioctl(uint32_t fd,uint32_t req,void* argp){
	return sys_call(SYS_IOCTL,fd,req,(uint32_t)argp,0,0);
}

static inline uint32_t sys_exit(uint32_t code){
	return sys_call(SYS_EXIT,code,0,0,0,0);
}


static inline uint32_t sys_fswait(uint32_t* fds,uint32_t cnt){
	return sys_call(SYS_FSWAIT,(uint32_t)fds,cnt,0,0,0);
}

static inline uint32_t sys_yield(){
	return sys_call(SYS_YIELD,0,0,0,0,0);
}

static inline uint32_t sys_sbrk(uint32_t size){
	return sys_call(SYS_SBRK,size,0,0,0,0);
}

static inline uint32_t sys_time(){
	return sys_call(SYS_TIME,0,0,0,0,0);
}

static inline uint32_t sys_waitpid(int pid){
	return sys_call(SYS_WAITPID,pid,0,0,0,0);
}

static inline char* sys_getcwd(char* buffer,uint32_t buffer_size){
	return (char*)sys_call(SYS_GETCWD,(uint32_t)buffer,buffer_size,0,0,0);
}

static inline uint32_t sys_chdir(char* path){
	return sys_call(SYS_CHDIR,(uint32_t)path,0,0,0,0);
}
static inline pid_t sys_getpid(){
	return sys_call(SYS_GETPID,0,0,0,0,0);
}
static inline uid_t sys_getuid(){
	return sys_call(SYS_GETUID,0,0,0,0,0);
}
static inline int sys_setuid(uid_t uid){
	return sys_call(SYS_SETUID,uid,0,0,0,0);
}
static inline int sys_link(char* whom,char* to){
	return sys_call(SYS_LINK,(uint32_t)whom,(uint32_t)to,0,0,0);
}
static inline int sys_send(uint32_t pid,uint32_t sig){
	return sys_call(SYS_SIG,pid,sig,0,0,0);
}
static inline int sys_signal(uint32_t sig, sig_handler_t handler){
	return sys_call(SYS_SIGHANDL,sig,(uint32_t)handler,0,0,0);
}
static inline int sys_sigexit(){
	return sys_call(SYS_SIGEXIT,0,0,0,0,0);
}

static inline int sys_sleep(uint32_t ticks){
	return sys_call(SYS_SLEEP,ticks,0,0,0,0);
}

static inline pid_t sys_getppid(){
	return sys_call(SYS_GETPPID,0,0,0,0,0);
}

static inline uint8_t sys_pipe(char* path,uint32_t buff_size){
	return sys_call(SYS_PIPE,(uint32_t)path,buff_size,0,0,0);
}
static inline uint8_t sys_pwreq(uint32_t req){
	return sys_call(SYS_PWREQ,req,0,0,0,0);
}

#endif
CH_END
