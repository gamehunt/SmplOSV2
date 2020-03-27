%macro ISR_NOERR 1
	global isr%1
	isr%1:
	  cli
	  push 0
	  push %1
	  jmp isr_stub
%endmacro

%macro ISR_ERR 1
	global isr%1
	isr%1:
	  cli
	  push %1
	  jmp isr_stub
%endmacro

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR 8
ISR_NOERR 9
ISR_ERR 10
ISR_ERR 11
ISR_ERR 12
ISR_ERR 13
ISR_ERR 14
ISR_NOERR 15
ISR_NOERR 16
ISR_NOERR 17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31
ISR_NOERR 127

extern fault_handler

isr_stub:
  cli

  pusha
  
  push ds
  push es
  push fs
  push gs

  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  cld

  push esp
  call fault_handler
  add esp , 4	

  pop ds
  pop es
  pop fs
  pop gs

  popa

  add esp, 8


  sti
  iret
