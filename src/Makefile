SRC=../src
BIN=../bin
BINSTAMP=$(BIN)/.dirstamp
IMAGE=$(BIN)/floppy.img
KERNEL=$(BIN)/kernel.bin
LD_SCRIPT=$(SRC)/kernel.lds
OBJS=kstart kmain util vga desc_table interrupt page_map acpi apic cpu_info \
	schedule
AS=as
CC=gcc
VM=qemu-system-x86_64
AS_OPTIONS=--64
CC_OPT=3
CC_OPTIONS=-m64 -c -fno-builtin -std=c99 -fno-stack-protector -O$(CC_OPT) \
	-Wall -DDEBUG
LD_OPTIONS=-melf_x86_64
VM_ACC=-no-kvm
VM_OPTIONS=$(VM_ACC) -smp sockets=1,cores=2,threads=2

_OBJS=$(patsubst %,$(BIN)/%.o,$(OBJS))

.PHONY: all run runt clean

all: $(IMAGE)

$(BINSTAMP):
	mkdir -p $(BIN)
	touch $@

$(BIN)/boot.o: $(SRC)/boot.s $(BINSTAMP)
	$(AS) $(AS_OPTIONS) $(SRC)/boot.s -o $(BIN)/boot.o

-include $(_OBJS:.o=.d)

$(BIN)/%.o: $(SRC)/%.s $(BINSTAMP)
	$(AS) $(AS_OPTIONS) $(SRC)/$*.s -o $(BIN)/$*.o

$(BIN)/%.o: $(SRC)/%.c $(BINSTAMP)
	$(CC) -c $(CC_OPTIONS) $(SRC)/$*.c -o $(BIN)/$*.o
	$(CC) -MM -MT '$(BIN)/$*.o' $(SRC)/$*.c > $(BIN)/$*.d

$(KERNEL): $(LD_SCRIPT) $(_OBJS)
	ld $(LD_OPTIONS) -T $(LD_SCRIPT) $(_OBJS) -o $(KERNEL)

$(IMAGE): $(BIN)/boot.o $(KERNEL)
	ld -Ttext 0x7C00 --oformat binary $(BIN)/boot.o -o $(IMAGE)
	dd if=$(KERNEL) of=$(IMAGE) seek=1 conv=sync

runt: $(IMAGE)
	screen $(VM) -fda $(IMAGE) -boot a $(VM_OPTIONS) -curses

run: $(IMAGE)
	$(VM) -fda $(IMAGE) -boot a $(VM_OPTIONS)

clean:
	rm -rf $(BIN)
