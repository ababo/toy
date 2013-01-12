IMAGE=floppy.img
KERNEL=kernel.bin
LD_SCRIPT=kernel.lds
OBJS=kstart.o kmain.o display.o util.o desc_table.o interrupt.o
CC_OPTIONS=-m64 -c -fno-builtin -std=c99 -fno-stack-protector
VM_OPTIONS=-no-kvm

image: boot kernel
	ld -Ttext 0x7C00 --oformat binary boot.o -o $(IMAGE)
	dd if=$(KERNEL) of=$(IMAGE) seek=1 conv=sync

kernel: $(LD_SCRIPT) $(OBJS)
	ld -T $(LD_SCRIPT) $(OBJS) -o $(KERNEL)

-include $(OBJS:.o=.d)

%.o: %.s
	as $*.s -o $*.o

%.o: %.c
	gcc -c $(CC_OPTIONS) $*.c -o $*.o
	gcc -MM $*.c > $*.d

boot: boot.s
	as boot.s -o boot.o

clean:
	rm -rf *.o *.d $(KERNEL) $(IMAGE)

runt: image
	screen qemu-system-x86_64 -fda $(IMAGE) -boot a $(VM_OPTIONS) -curses

run: image
	qemu-system-x86_64 -fda $(IMAGE) -boot a $(VM_OPTIONS)