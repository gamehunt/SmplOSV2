
ARCHDIR=arch/$(ARCH)

KERNEL_OBJS=\
$(ARCHDIR)/boot.o\
kernel.o\
io/io.o\
dev/serial.o\
dev/eld.o \
misc/log.o\
misc/panic.o\
misc/stat.o\
$(ARCHDIR)/mem/memstat.o\
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
$(ARCHDIR)/interrupts/syscall.o\
dev/pci.o\
dev/pit.o\
dev/cmos.o\
dev/rtc.o\
fs/vfs.o\
fs/tar.o\
$(ARCHDIR)/proc/proc_asm.o\
$(ARCHDIR)/proc/proc.o\
module/symtable.o\
module/module.o\
module/ramdisk.o\
module/elf.o\

KERNEL_LINK_OBJS=\
$(KERNEL_OBJS)\
$(KERNEL_LINKS) \

.PHONY: all clean install install-headers
.SUFFIXES: .o .c .S .asm

all: smplos.kernel

smplos.kernel:$(KERNEL_OBJS) $(ARCHDIR)/linker.ld
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
	
