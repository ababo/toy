IMAGE=floppy.img
KERNEL=kernel.bin
LD_SCRIPT=kernel.lds
OBJS=kstart.o kmain.o display.o util.o desc_table.o interrupt.o mem_map.o \
	page_map.o apic.o acpi.o
CC=gcc
VM=qemu-system-x86_64
AS_OPTIONS=--64
OPT=3
CC_OPTIONS=-m64 -c -fno-builtin -std=c99 -fno-stack-protector -O$(OPT) -Wall
LD_OPTIONS=-melf_x86_64
VM_OPTIONS=-no-kvm

image: boot kernel
	ld -Ttext 0x7C00 --oformat binary boot.o -o $(IMAGE)
	dd if=$(KERNEL) of=$(IMAGE) seek=1 conv=sync

kernel: $(LD_SCRIPT) $(OBJS)
	ld $(LD_OPTIONS) -T $(LD_SCRIPT) $(OBJS) -o $(KERNEL)

-include $(OBJS:.o=.d)

%.o: %.s
	as $(AS_OPTIONS) $*.s -o $*.o

%.o: %.c
	$(CC) -c $(CC_OPTIONS) $*.c -o $*.o
	$(CC) -MM $*.c > $*.d

boot: boot.s
	as $(AS_OPTIONS) boot.s -o boot.o

clean:
	rm -rf *.o *.d $(KERNEL) $(IMAGE)

runt: image
	screen $(VM) -fda $(IMAGE) -boot a $(VM_OPTIONS) -curses

run: image
	$(VM) -fda $(IMAGE) -boot a $(VM_OPTIONS)
