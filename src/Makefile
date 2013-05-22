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

BIN = ../bin
BIN_ARCH = $(BIN)/$(ARCH)
BIN_STAMP = $(BIN)/.stamp

OBJS = $(patsubst $(SRC)/%.c,$(BIN)/%.o,$(wildcard $(SRC)/*.c))
OBJS_ARCH = $(patsubst $(SRC_ARCH)/%.c,$(BIN_ARCH)/%.o, \
  $(wildcard $(SRC_ARCH)/*.c))

ifeq ($(ARCH),x86_64)

LD_OPTIONS += -z max-page-size=0x1

VM = qemu-system-x86_64
VM_ACC = -no-kvm
VM_OPTIONS = $(VM_ACC) -smp sockets=1,cores=2,threads=2

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
-include $(OBJS_ARCH:.o=.d)

$(BOOT32_OBJ): $(BOOT32_SRC) $(BIN_STAMP)
	$(CC) -m32 -c $(CC_OPTIONS) $(BOOT32_SRC) -o $(BOOT32_OBJ)
	$(CC) -m32 -MM -MT '$(BOOT32_OBJ)' $(BOOT32_SRC) > $(BIN)/$*.d
	objcopy -O elf64-x86-64 $(BOOT32_OBJ)

$(BIN_ARCH)/%.o: $(SRC_ARCH)/%.c $(BIN_STAMP)
	$(CC) -m64 -c $(CC_OPTIONS) $(SRC_ARCH)/$*.c -o $(BIN_ARCH)/$*.o
	$(CC) -m64 -MM -MT '$(BIN_ARCH)/$*.o' $(SRC_ARCH)/$*.c > \
	  $(BIN_ARCH)/$*.d

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
	$(CC) -m64 -MM -MT '$(BIN)/$*.o' $(SRC)/$*.c > $(BIN)/$*.d

$(BIN_STAMP):
	mkdir -p $(BIN_ARCH)
	touch $@

clean:
	rm -rf $(BIN)