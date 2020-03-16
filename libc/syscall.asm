global sys_call

sys_call:
	push ebp
	mov ebp,esp

	push ebx
	push ecx
	push edx
	push esi
	push edi

	mov eax,[ebp+8 ] 
	mov ebx,[ebp+12]
	mov ecx,[ebp+16]
	mov edx,[ebp+20]
	mov esi,[ebp+24]
	mov edi,[ebp+28]

	int 0x7f

	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx

	leave
	ret
	
