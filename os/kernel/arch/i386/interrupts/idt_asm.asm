global idt_load
extern idtp


idt_load:							;loads the idt

cli
lidt[idtp+2]					;idtp points to the idt
sti
ret


