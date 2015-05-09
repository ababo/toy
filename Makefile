ARCH := x86_64
OPT := 3

ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
KERNEL_DIR := $(ROOT_DIR)/kernel
ARCH_DIR := $(KERNEL_DIR)/$(ARCH)
BUILD_DIR := $(ROOT_DIR)/build

LDS := $(ARCH_DIR)/kernel.lds
BUILD_STAMP := $(BUILD_DIR)/.stamp
KERNEL := $(BUILD_DIR)/kernel.elf

AS := as
AS_FLAGS :=

AS_SRCS := $(wildcard $(ARCH_DIR)/*.s)
AS_OBJS := $(patsubst $(ARCH_DIR)/%.s, $(BUILD_DIR)/%.o, $(AS_SRCS))

RUSTC := rustc
RUSTC_EMIT := --crate-type lib --emit obj
RUSTC_TGT := --target $(ARCH)-unknown-linux-gnu
RUSTC_OPT := -C opt-level=$(OPT)
RUSTC_CFG := --cfg arch_$(ARCH)
RUSTC_FLAGS := $(RUSTC_EMIT) $(RUSTC_TGT) $(RUSTC_OPT) $(RUSTC_CFG)

RUST_SRCS := $(KERNEL_DIR)/*.rs $(ARCH_DIR)/*.rs
RUST_OBJ := $(BUILD_DIR)/kernel.o

LD :=
LD_SCRIPT := -T $(LDS)
LD_BARE := -nostdlib --build-id=none -z max-page-size=4096
LD_FLAGS := $(LD_BARE) $(LD_SCRIPT)

EMU := qemu-system-$(ARCH)
EMU_UI := #-nographic
EMU_FLAGS := -kernel $(KERNEL) $(EMU_UI)

HOST := $(shell uname)
ifeq ($(HOST), Linux)
AS := $(ARCH)-linux-gnu-as
LD := $(ARCH)-linux-gnu-ld
endif
ifeq ($(HOST), Darwin)
AS := $(ARCH)-elf-as
LD := $(ARCH)-elf-ld
endif

.PHONY: all clean run

all: $(KERNEL)

clean:
	@echo Removing $(BUILD_DIR)
	@rm -rf $(BUILD_DIR)

$(BUILD_DIR)/.stamp:
	@echo Creating $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)
	@touch $(BUILD_STAMP)

$(AS_OBJS): $(AS_SRCS) $(BUILD_STAMP)
	@echo Assembling $(abspath $@)
	@$(AS) $(AS_FLAGS) -o $@ $<

$(RUST_OBJ): $(BUILD_STAMP) $(RUST_SRCS)
	@echo Compiling $(abspath $@)
	@$(RUSTC) $(RUSTC_FLAGS) $(KERNEL_DIR)/lib.rs -o $(RUST_OBJ)

$(KERNEL): $(LDS) $(AS_OBJS) $(RUST_OBJ)
	@echo Linking $(KERNEL)
	@$(LD) $(LD_FLAGS) -o $(KERNEL) $(AS_OBJS) $(RUST_OBJ)

run: $(KERNEL)
	@echo "Running QEMU (to exit press Ctrl-a x)"
	@$(EMU) $(EMU_FLAGS)