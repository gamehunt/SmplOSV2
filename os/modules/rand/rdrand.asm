
global is_rdrand_available
global get_random

is_rdrand_available:
	push ebp
    mov  ebp, esp
	mov eax, 1     ; set EAX to request function 1
	mov ecx, 0     ; set ECX to request subfunction 0
	cpuid
	shr ecx, 30    ; the result we want is in ECX...
	and ecx, 1     ; ...test for the flag of interest...
	mov eax,ecx
	mov  esp, ebp
	pop ebp
    ret

get_random:
	push ebp
    mov  ebp, esp
	rdrand eax
	mov  esp, ebp
	pop ebp
    ret
