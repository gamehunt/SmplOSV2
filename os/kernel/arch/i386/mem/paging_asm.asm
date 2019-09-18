global enable_paging
global disable_paging
;extern k_pde

global set_page_directory

set_page_directory:
 push ebp
 mov  ebp, esp


 mov eax, [ebp+8]
 mov cr3, eax

 mov esp, ebp
 pop ebp
 
 ret

enable_paging:  

 mov eax,cr0
 or eax, 0x80000001
 mov cr0, eax
 
 ret

disable_paging:  

 mov eax,cr0
 and eax, 0x80000000
 mov cr0, eax
 
 ret
