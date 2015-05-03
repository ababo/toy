ARCH=x86_64
OPT=3

.PHONY: all clean run

ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
BUILD_DIR := $(ROOT_DIR)/build

all: $(BUILD_DIR)/kernel.elf

clean:
	@echo Removing build directory...
	@rm -rf $(BUILD_DIR)

$(BUILD_DIR)/.stamp:
	@echo Creating build directory...
	@mkdir -p $(BUILD_DIR)
	@touch $(BUILD_DIR)/.stamp

KERNEL_DIR := $(ROOT_DIR)/kernel
ARCH_DIR := $(KERNEL_DIR)/$(ARCH)

RUSTC := rustc
RUSTC_EMIT := --crate-type lib --emit obj
RUSTC_TGT := --target $(ARCH)-unknown-linux-gnu
RUSTC_OPT := -C opt-level=$(OPT)
RUSTC_CFG := --cfg arch_$(ARCH)
RUSTC_FLAGS := $(RUSTC_EMIT) $(RUSTC_TGT) $(RUSTC_OPT) $(RUSTC_CFG)

$(BUILD_DIR)/kernel.o: $(BUILD_DIR)/.stamp $(KERNEL_DIR)/*.rs $(ARCH_DIR)/*.rs
	@echo Compiling kernel...
	@$(RUSTC) $(RUSTC_FLAGS) $(KERNEL_DIR)/lib.rs -o $(BUILD_DIR)/kernel.o

LD=$(ARCH)-elf-ld
LD_BARE := -nostdlib --build-id=none -z max-page-size=8
LD_SCRIPT := -T $(ARCH_DIR)/kernel.lds
LD_FLAGS := $(LD_BARE) $(LD_SCRIPT)

$(BUILD_DIR)/kernel.elf: $(BUILD_DIR)/kernel.o $(ARCH_DIR)/kernel.lds
	@echo Linking kernel...
	@$(LD) $(LD_FLAGS) -o $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.o