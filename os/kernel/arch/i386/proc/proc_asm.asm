GLOBAL jump_usermode ;you may need to remove this _ to work right.. 
jump_usermode:
     push ebp
     mov  ebp, esp


     mov ax,0x23
     mov ds,ax
     mov es,ax 
     mov fs,ax 
     mov gs,ax ;we don't need to worry about SS. it's handled by iret
 
     push 0x23 ;user data segment with bottom 2 bits set for ring 3
     push 0xC0001000 ;push our current ss for the iret stack frame
     pushf
     pop eax
     or eax, 0x200
     push eax
     push 0x1B; ;user code segment with bottom 2 bits set for ring 3
     push dword [ebp+8] ;may need to remove the _ for this to work right 
     iret
     pop ebp
     ret
