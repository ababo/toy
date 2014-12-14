#
# Makefile for x86_64-specific kernel code.
#

ARCH := x86-64

include ../common.mk

SRC_DIR := $(ROOT_DIR)/x86-64
TGT_DIR := $(BUILD_DIR)/x86-64
TGT_STAMP := $(TGT_DIR)/.stamp

SRCS := $(wildcard *.c) $(wildcard *.S)
OBJS := $(patsubst %.c, $(TGT_DIR)/%.o, $(SRCS))
OBJS := $(patsubst %.S, $(TGT_DIR)/%.o, $(OBJS))

ARCH := x86-64

.PHONY: all clean

all: $(OBJS)

$(TGT_STAMP):
	@echo Creating $(TGT_DIR)
	@mkdir -p $(TGT_DIR)
	@touch $(TGT_STAMP)

-include $(OBJS:.o=.d)

$(TGT_DIR)/%.o: %.c $(TGT_STAMP)
	@echo Compiling $(abspath $<)
	@$(CC) $(CC_FLAGS) -o $@ $<
	@$(CC) -MM -MT $@ $(CC_FLAGS) $< > $(TGT_DIR)/$*.d

$(TGT_DIR)/%.o: %.S $(TGT_STAMP)
	@echo Compiling $(abspath $<)
	@$(AS) $(AS_FLAGS) -o $@ $<
	@$(AS) -MM -MT $@ $(AS_FLAGS) $< > $(TGT_DIR)/$*.d

clean:
	@echo Removing $(TGT_DIR)
	@rm -rf $(TGT_DIR)
