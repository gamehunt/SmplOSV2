global gdt_load
extern gp

gdt_load:   
    cli         ;enables GDT
    lgdt [gp+2]       ;pointer to structure + 2 because of padding (16 bit)
    mov eax,0x10			;pointer to the data segment
    mov ds,eax
    mov es,eax
    mov fs,eax
    mov gs,eax
    mov ss,eax
    mov eax, cr0 
    or al, 1       ; set PE (Protection Enable) bit in CR0 (Control Register 0)
    mov cr0, eax
    jmp 0x08:cschange		;far jump to the code segment, changes CS pointer

cschange:
    ret   



global tss_flush

tss_flush:
    mov ax, 0x28  ;The descriptor of the TSS in the GDT (e.g. 0x28 if the sixths entry in your GDT describes your TSS)
    ltr ax        ;The actual load
    ret

