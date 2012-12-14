IMAGE=floppy.img
KERNEL=kernel.bin
LD_SCRIPT=kernel.lds
OBJS=kstart.o kmain.o display.o

run: image
	screen qemu-system-x86_64 -fda $(IMAGE) -boot a -no-kvm -curses

image: boot kernel
	ld -Ttext 0x7C00 --oformat binary boot.o -o $(IMAGE)
	cat $(KERNEL) >> $(IMAGE)

kernel: $(LD_SCRIPT) $(OBJS)
	ld -T $(LD_SCRIPT) $(OBJS) -o $(KERNEL)

-include $(OBJS:.o=.d)

%.o: %.s
	as $*.s -o $*.o

%.o: %.c
	gcc -c -m64 -c -fno-builtin $*.c -o $*.o
	gcc -MM $*.c > $*.d

boot: boot.s
	as boot.s -o boot.o

clean:
	rm -rf *.o *.d $(KERNEL) $(IMAGE)
