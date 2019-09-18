
ARCHDIR=arch/$(ARCH)

CRTI_OBJS=\
$(ARCHDIR)/crti.o \
$(ARCHDIR)/crtbegin.o \

CRTN_OBJS=\
$(ARCHDIR)/crtend.o \
$(ARCHDIR)/crtn.o \


KERNEL_OBJS=\
$(ARCHDIR)/boot.o\
kernel.o\
$(ARCHDIR)/io/terminal.o\
io/io.o\
misc/log.o\
misc/panic.o\
$(ARCHDIR)/mem/gdt.o\
$(ARCHDIR)/mem/gdt_asm.o \
$(ARCHDIR)/mem/pmm.o \
$(ARCHDIR)/mem/paging.o \
$(ARCHDIR)/mem/paging_asm.o \
$(ARCHDIR)/mem/kheap.o \
$(ARCHDIR)/interrupts/idt.o\
$(ARCHDIR)/interrupts/idt_asm.o\
$(ARCHDIR)/interrupts/irq.o\
$(ARCHDIR)/interrupts/isr_asm.o\
$(ARCHDIR)/interrupts/isr.o\
$(ARCHDIR)/interrupts/irq_asm.o\
$(ARCHDIR)/interrupts/pic.o\
dev/pci.o\
dev/pit.o\
dev/cmos.o\
dev/rtc.o\
dev/tty.o\
fs/vfs.o\
$(ARCHDIR)/proc/sched_asm.o\
$(ARCHDIR)/proc/sched.o\

KERNEL_LINK_OBJS=\
$(CRTI_OBJS)\
$(KERNEL_OBJS)\
$(KERNEL_LINKS) \
$(CRTN_OBJS)\

.PHONY: all clean install install-headers
.SUFFIXES: .o .c .S .asm

all: smplos.kernel

smplos.kernel: $(CRTI_OBJS) $(KERNEL_OBJS) $(CRTN_OBJS) $(ARCHDIR)/linker.ld
	$(CC) -T $(ARCHDIR)/linker.ld -o $@ $(KERNEL_CFLAGS) $(KERNEL_LINK_OBJS) 
	grub-file --is-x86-multiboot smplos.kernel

.c.o: 
	$(CC) -MD -c $< -o $@ -std=gnu11 $(KERNEL_CFLAGS) -Iinclude -D__smplos_kernel -D'ARCH="$(ARCH)"' -D'__ARCH=$(ARCH)'
.asm.o:
	nasm -f elf32 $< -o $@
.S.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 $(KERNEL_CFLAGS) -Iinclude -D__smplos_kernel -D'ARCH="$(ARCH)"' -D'__ARCH=$(ARCH)'

$(ARCHDIR)/crtbegin.o $(ARCHDIR)/crtend.o:
	OBJ=`$(CC) $(KERNEL_CFLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@
clean:
	rm -f smplos.kernel
	rm -f $(SYSROOT)/boot/kernel.bin
	rm -f $(KERNEL_OBJS) *.o */*.o */*/*.o
	rm -f $(KERNEL_OBJS:.o=.d) *.d */*.d */*/*.d
	rm -f $(CRTI_OBJS) *.o */*.o */*/*.o
	rm -f $(CRTI_OBJS:.o=.d) *.d */*.d */*/*.d
	rm -f $(CRTN_OBJS) *.o */*.o */*/*.o
	rm -f $(CRTN_OBJS:.o=.d) *.d */*.d */*/*.d
install:
	cp smplos.kernel $(SYSROOT)/boot/kernel.bin

install-headers:
	rsync -a include/ $(SYSROOT)$(SYSINC)/
	
