GLOBAL jump_usermode  
jump_usermode:
     push ebp
     mov  ebp, esp


     mov ax,0x23
     mov ds,ax
     mov es,ax 
     mov fs,ax 
     mov gs,ax ;we don't need to worry about SS. it's handled by iret
 
     push 0x23 ;user data segment with bottom 2 bits set for ring 3
     
     mov eax, esp
     
     push eax ;push our current ss for the iret stack frame
     pushf
     pop eax
     or eax, 0x200
     push eax
     push 0x1B; ;user code segment with bottom 2 bits set for ring 3
     push dword [ebp+8] 
     iret
     pop ebp
     ret
