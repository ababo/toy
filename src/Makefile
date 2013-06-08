ARCH = x86_64

CC = gcc
CC_OPT = 3
CC_WARN = -Wall -Wextra
CC_OPTIONS = -c -std=c99 -fno-stack-protector -O$(CC_OPT) $(CC_WARN) -DDEBUG \
  -D ARCH=$(ARCH)

LD = ld
LD_SCRIPT = $(SRC_ARCH)/kernel.lds
LD_OPTIONS =

SRC = ../src
SRC_ARCH = $(SRC)/$(ARCH)
SRC_TEST = $(SRC)/test

BIN = ../bin
BIN_ARCH = $(BIN)/$(ARCH)
BIN_TEST = $(BIN)/test
BIN_STAMP = $(BIN)/.stamp

OBJS = $(patsubst $(SRC)/%.c,$(BIN)/%.o,$(wildcard $(SRC)/*.c))
OBJS += $(patsubst $(SRC_TEST)/%.c,$(BIN_TEST)/%.o,$(wildcard $(SRC_TEST)/*.c))
OBJS_ARCH = $(patsubst $(SRC_ARCH)/%.c,$(BIN_ARCH)/%.o, \
  $(wildcard $(SRC_ARCH)/*.c))

ifeq ($(ARCH), x86_64)

ifeq ($(CC), clang) # integrated assembler does not yet support .code16
CC_OPTIONS += -no-integrated-as
endif

LD_OPTIONS += -z max-page-size=0x1

VM = qemu-system-x86_64
VM_ACC = -no-kvm
VM_DRIVES = -device ahci,id=ahci -drive id=disk,file=toy.qcow2,if=none \
  -device ide-drive,drive=disk,bus=ahci.0
VM_SMP = -smp sockets=1,cores=2,threads=2
VM_OPTIONS = $(VM_ACC) $(VM_DRIVES) $(SMP)

GRUB_CONF = $(SRC_ARCH)/grub.cfg
IMAGE = $(BIN)/toy.iso
KERNEL = $(BIN)/toy.bin

ISO_GRUB = $(BIN)/iso/boot/grub
ISO_BOOT = $(patsubst %/,%,$(dir $(ISO_GRUB)))

BOOT32_SRC = $(SRC_ARCH)/boot32.c
BOOT32_OBJ = $(BIN_ARCH)/boot32.o

OBJS += $(filter-out $(BOOT32_OBJ),$(OBJS_ARCH))

.PHONY: all run runt

all: $(IMAGE)

-include $(BOOT32_OBJ:.o=.d)

$(BOOT32_OBJ): $(BOOT32_SRC) $(BIN_STAMP)
	$(CC) -m32 -c $(CC_OPTIONS) $(BOOT32_SRC) -o $(BOOT32_OBJ)
	$(CC) -m32 -MM -MT '$(BOOT32_OBJ)' $(CC_OPTIONS) $(BOOT32_SRC) > \
	  $(BIN)/$*.d
	objcopy -O elf64-x86-64 $(BOOT32_OBJ)

$(BIN_ARCH)/%.o: $(SRC_ARCH)/%.c $(BIN_STAMP)
	$(CC) -m64 -c $(CC_OPTIONS) $(SRC_ARCH)/$*.c -o $(BIN_ARCH)/$*.o
	$(CC) -m64 -MM -MT '$(BIN_ARCH)/$*.o' $(CC_OPTIONS) \
          $(SRC_ARCH)/$*.c > $(BIN_ARCH)/$*.d

$(KERNEL): $(LD_SCRIPT) $(BOOT32_OBJ) $(OBJS)
	cpp -E -P $(LD_SCRIPT) | ld $(LD_OPTIONS) -T /dev/stdin \
          $(BOOT32_OBJ) $(OBJS) -o $(KERNEL)

$(IMAGE): $(GRUB_CONF) $(KERNEL)
	mkdir -p $(ISO_GRUB)
	cp $(GRUB_CONF) $(ISO_GRUB)/
	cp $(KERNEL) $(ISO_BOOT)
	grub-mkrescue -o $(IMAGE) --diet $(dir $(ISO_BOOT))

runt: $(IMAGE)
	screen $(VM) -cdrom $(IMAGE) -boot d $(VM_OPTIONS) -curses

run: $(IMAGE)
	$(VM) -cdrom $(IMAGE) -boot d $(VM_OPTIONS)

endif # ifeq ($(ARCH),x86_64)

.PHONY: clean

-include $(OBJS:.o=.d)

$(BIN)/%.o: $(SRC)/%.c $(BIN_STAMP)
	$(CC) -m64 -c $(CC_OPTIONS) $(SRC)/$*.c -o $(BIN)/$*.o
	$(CC) -m64 -MM -MT '$(BIN)/$*.o' $(CC_OPTIONS) $(SRC)/$*.c > \
          $(BIN)/$*.d

$(BIN_TEST)/%.o: $(SRC_TEST)/%.c $(BIN_STAMP)
	$(CC) -m64 -c $(CC_OPTIONS) $(SRC_TEST)/$*.c -o $(BIN_TEST)/$*.o
	$(CC) -m64 -MM -MT '$(BIN_TEST)/$*.o' $(CC_OPTIONS) \
          $(SRC_TEST)/$*.c > $(BIN_TEST)/$*.d

$(BIN_STAMP):
	mkdir -p $(BIN_ARCH)
	mkdir -p $(BIN_TEST)
	touch $@

clean:
	rm -rf $(BIN)