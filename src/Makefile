SRC=../src
BIN=../bin
BINSTAMP=$(BIN)/.dirstamp
IMAGE=$(BIN)/toy.iso
KERNEL=$(BIN)/toy.bin
ISO_GRUB=$(BIN)/iso/boot/grub
LD_SCRIPT=$(SRC)/kernel.lds
GRUB_CONF=$(SRC)/grub.cfg
OBJS=boot kmain
CC=gcc
VM=qemu-system-x86_64
CC_OPT=3
CC_OPTIONS=-m64 -c -fno-builtin -std=c99 -fno-stack-protector -O$(CC_OPT) \
	-Wall -DDEBUG
LD_OPTIONS=-z max-page-size=0x0
VM_ACC=-no-kvm
VM_OPTIONS=$(VM_ACC) -smp sockets=1,cores=2,threads=2

_OBJS=$(patsubst %,$(BIN)/%.o,$(OBJS))
_ISO_BOOT=$(patsubst %/,%,$(dir $(ISO_GRUB)))

.PHONY: all run runt clean

all: $(IMAGE)

$(BINSTAMP):
	mkdir -p $(BIN)
	touch $@

-include $(_OBJS:.o=.d)

$(BIN)/%.o: $(SRC)/%.S $(BINSTAMP)
	$(CC) $(CC_OPTIONS) $(SRC)/$*.S -o $(BIN)/$*.o

$(BIN)/%.o: $(SRC)/%.c $(BINSTAMP)
	$(CC) -c $(CC_OPTIONS) $(SRC)/$*.c -o $(BIN)/$*.o
	$(CC) -MM -MT '$(BIN)/$*.o' $(SRC)/$*.c > $(BIN)/$*.d

$(KERNEL): $(LD_SCRIPT) $(_OBJS)
	ld $(LD_OPTIONS) -T $(LD_SCRIPT) $(_OBJS) -o $(KERNEL)

$(IMAGE): $(GRUB_CONF) $(KERNEL)
	mkdir -p $(ISO_GRUB)
	cp $(GRUB_CONF) $(ISO_GRUB)/
	cp $(KERNEL) $(_ISO_BOOT)
	grub-mkrescue -o $(IMAGE) --diet $(dir $(_ISO_BOOT))

runt: $(IMAGE)
	screen $(VM) -cdrom $(IMAGE) -boot d $(VM_OPTIONS) -curses

run: $(IMAGE)
	$(VM) -cdrom $(IMAGE) -boot d $(VM_OPTIONS)

clean:
	rm -rf $(BIN)