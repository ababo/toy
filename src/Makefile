SRC=../src
BIN=../bin
BIN_STAMP=$(BIN)/.stamp
IMAGE=$(BIN)/toy.iso
KERNEL=$(BIN)/toy.bin
ISO_GRUB=$(BIN)/iso/boot/grub
LD_SCRIPT=$(SRC)/kernel.LDS
GRUB_CONF=$(SRC)/grub.cfg
BOOT32_OBJ=boot32
OBJS=kmain
CC=gcc
VM=qemu-system-x86_64
CC_OPT=3
CC_OPTIONS=-c -fno-builtin -std=c99 -fno-stack-protector -O$(CC_OPT) -Wall \
           -DDEBUG
LD_OPTIONS=-z max-page-size=0x1
VM_ACC=-no-kvm
VM_OPTIONS=$(VM_ACC) -smp sockets=1,cores=2,threads=2

_BOOT32_SRC=$(patsubst %,$(SRC)/%.c,$(BOOT32_OBJ))
_BOOT32_OBJ=$(patsubst %,$(BIN)/%.o,$(BOOT32_OBJ))
_OBJS=$(patsubst %,$(BIN)/%.o,$(OBJS))
_ISO_BOOT=$(patsubst %/,%,$(dir $(ISO_GRUB)))

.PHONY: all run runt clean

all: $(IMAGE)

$(BIN_STAMP):
	mkdir -p $(BIN)
	touch $@

-include $(_BOOT32_OBJ:.o=.d)
-include $(_OBJS:.o=.d)

$(_BOOT32_OBJ): $(_BOOT32_SRC) $(BIN_STAMP)
	$(CC) -m32 -c $(CC_OPTIONS) $(_BOOT32_SRC) -o $(_BOOT32_OBJ)
	$(CC) -m32 -MM -MT '$(_BOOT32_OBJ)' $(_BOOT32_SRC) > $(BIN)/$*.d
	objcopy -O elf64-x86-64 $(_BOOT32_OBJ)

$(BIN)/%.o: $(SRC)/%.c $(BIN_STAMP)
	$(CC) -m64 -c $(CC_OPTIONS) $(SRC)/$*.c -o $(BIN)/$*.o
	$(CC) -m64 -MM -MT '$(BIN)/$*.o' $(SRC)/$*.c > $(BIN)/$*.d

$(KERNEL): $(LD_SCRIPT) $(_BOOT32_OBJ) $(_OBJS)
	cpp -E -P $(LD_SCRIPT) | ld $(LD_OPTIONS) -T /dev/stdin \
          $(_BOOT32_OBJ) $(_OBJS) -o $(KERNEL)

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