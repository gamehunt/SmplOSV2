
ARCHDIR=arch/$(ARCH)
ACPICA_DIR=dev/acpica
ACPICA_SOURCES:=$(shell find $(ACPICA_DIR) -name '*.c')
ACPICA_OBJS=$(ACPICA_SOURCES:%.c=%.acpica.o)

KERNEL_OBJS=\
$(ARCHDIR)/boot.o\
kernel.o\
io/io.o\
dev/serial.o\
dev/eld.o \
misc/log.o\
misc/panic.o\
misc/stat.o\
misc/types/queue.o\
misc/types/dyn_array.o\
$(ARCHDIR)/mem/memstat.o\
$(ARCHDIR)/mem/gdt.o\
$(ARCHDIR)/mem/gdt_asm.o \
$(ARCHDIR)/mem/pmm.o \
$(ARCHDIR)/mem/paging.o \
$(ARCHDIR)/mem/paging_asm.o \
$(ARCHDIR)/mem/kheap.o \
$(ARCHDIR)/interrupts/idt.o\
$(ARCHDIR)/interrupts/idt_asm.o\
$(ARCHDIR)/interrupts/isr.o\
$(ARCHDIR)/interrupts/isr_asm.o\
$(ARCHDIR)/interrupts/irq.o\
$(ARCHDIR)/interrupts/irq_asm.o\
$(ARCHDIR)/interrupts/pic.o\
$(ARCHDIR)/interrupts/syscall.o\
dev/pci.o\
dev/pit.o\
dev/cmos.o\
dev/rtc.o\
fs/vfs.o\
fs/tar.o\
fs/pipe.o\
$(ARCHDIR)/proc/proc_asm.o\
$(ARCHDIR)/proc/proc.o\
$(ARCHDIR)/proc/sync.o\
module/symtable.o\
module/module.o\
module/ramdisk.o\
module/elf.o\

KERNEL_LINK_OBJS=\
$(KERNEL_OBJS)\
$(ACPICA_OBJS) \
$(KERNEL_LINKS) \

.PHONY: all clean install install-headers
.SUFFIXES: .o .c .acpica.o .S .asm

all: smplos.kernel

smplos.kernel:$(KERNEL_OBJS) $(ACPICA_OBJS) $(ARCHDIR)/linker.ld
	$(CC) -T $(ARCHDIR)/linker.ld -o $@ $(KERNEL_CFLAGS) $(KERNEL_LINK_OBJS) 
	grub-file --is-x86-multiboot smplos.kernel

.c.o: 
	$(CC) -MD -c $< -o $@ -std=gnu11 $(KERNEL_CFLAGS) -Iinclude -D__smplos_kernel -D'ARCH="$(ARCH)"' -D'__ARCH=$(ARCH)'

.c.acpica.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 $(KERNEL_CFLAGS) -Iinclude -Iinclude/kernel/dev/acpica -Iinclude/kernel/dev/acpica/platform -D__smplos_kernel -D'ARCH="$(ARCH)"' -D'__ARCH=$(ARCH)'

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
	rm -f $(ACPICA_OBJS) *.acpica.o */*.acpica.o
	rm -f $(ACPICA_OBJS:.acpica.o=.d) *.d */*.d
install:
	cp smplos.kernel $(SYSROOT)/boot/kernel.bin

install-headers:
	rsync -a include/ $(SYSROOT)$(SYSINC)/
	
