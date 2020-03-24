GLOBAL _start
EXTERN libc_init
_start:
	push 0
	push 0
	call libc_init 
