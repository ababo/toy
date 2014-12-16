#
# Kernel makefile.
#

ARCH := x86-64

ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
BUILD_DIR := $(ROOT_DIR)/build

SRC_DIR := $(ROOT_DIR)
TGT_DIR := $(BUILD_DIR)
TGT_STAMP := $(TGT_DIR)/.stamp

CC := clang
CC_LANG := -std=c99
CC_WARN := -pedantic -W -Wall
CC_BARE := -fno-stack-protector -nostdinc -ffreestanding
CC_INCLUDES := -I $(ROOT_DIR)
CC_OPTIMIZE := -O3
CC_FLAGS = -c $(CC_LANG) $(CC_WARN) $(CC_BARE) $(CC_INCLUDES) $(CC_OPTIMIZE)

AS := $(CC)
AS_INCLUDES := $(CC_INCLUDES)
AS_FLAGS := -c $(AS_INCLUDES)

LD :q= ld
LD_BARE := -nostdlib --build-id=none -z max-page-size=0x1
LD_SCRIPT := -T $(ARCH)/kernel.ld
LD_OPTIONS := $(LD_BARE) $(LD_SCRIPT)

ifeq ($(ARCH), x86-64)
	CC_FLAGS += -m64
	AS_FLAGS += -m64
endif

SRCS := $(wildcard *.c)
ARCH_SRCS := $(wildcard $(ARCH)/*.c) $(wildcard $(ARCH)/*.S)

OBJS := $(patsubst %.c, $(TGT_DIR)/%.o, $(SRCS))
ARCH_OBJS := $(patsubst %.c, $(TGT_DIR)/%.o, $(ARCH_SRCS))
ARCH_OBJS := $(patsubst %.S, $(TGT_DIR)/%.o, $(ARCH_OBJS))

KERNEL := $(TGT_DIR)/kernel

.PHONY: all clean

all: $(KERNEL)

$(KERNEL): $(OBJS) $(ARCH_OBJS)
	@echo Linking $(abspath $(KERNEL))
	@$(LD) $(LD_OPTIONS) -o $(KERNEL) $(OBJS) $(ARCH_OBJS)

$(TGT_STAMP):
	@echo Creating $(TGT_DIR)
	@mkdir -p $(TGT_DIR)/$(ARCH)
	@touch $(TGT_STAMP)

-include $(OBJS:.o=.d)
-include $(ARCH_OBJS:.o=.d)

$(TGT_DIR)/%.o: %.c $(TGT_STAMP)
	@echo Compiling $(abspath $<)
	@$(CC) $(CC_FLAGS) -o $@ $<
	@$(CC) -MM -MT $@ $(CC_FLAGS) $< > $(TGT_DIR)/$*.d

$(TGT_DIR)/$(ARCH)/%.o: $(ARCH)/%.c $(TGT_STAMP)
	@echo Compiling $(abspath $<)
	@$(CC) $(CC_FLAGS) -o $@ $<
	@$(CC) -MM -MT $@ $(CC_FLAGS) $< > $(TGT_DIR)/$(ARCH)/$*.d

$(TGT_DIR)/$(ARCH)/%.o: $(ARCH)/%.S $(TGT_STAMP)
	@echo Compiling $(abspath $<)
	@$(AS) $(AS_FLAGS) -o $@ $<
	@$(AS) -MM -MT $@ $(AS_FLAGS) $< > $(TGT_DIR)/$(ARCH)/$*.d

clean:
	@echo Removing $(TGT_DIR)
	@rm -rf $(TGT_DIR)
