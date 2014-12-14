#
# Kernel makefile.
#

ARCH := x86-64

include common.mk

SRC_DIR := $(ROOT_DIR)
TGT_DIR := $(BUILD_DIR)
TGT_STAMP := $(TGT_DIR)/.stamp

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c, $(TGT_DIR)/%.o, $(SRCS))
ARCH_OBJS := `find $(BUILD_DIR)/$(ARCH) -name '*.o'`

KERNEL := $(TGT_DIR)/kernel

.PHONY: all clean

all: $(KERNEL)

$(KERNEL): $(OBJS)
	@echo Linking $(abspath $(KERNEL))
	@cd $(ARCH) && make -s
	$(LD) $(LD_OPTIONS) -o $(KERNEL) $(OBJS) $(ARCH_OBJS)

$(TGT_STAMP):
	@echo Creating $(TGT_DIR)
	@mkdir -p $(TGT_DIR)
	@touch $(TGT_STAMP)

-include $(OBJS:.o=.d)

$(TGT_DIR)/%.o: %.c $(TGT_STAMP)
	@echo Compiling $(abspath $<)
	@$(CC) $(CC_FLAGS) -o $@ $<
	@$(CC) -MM -MT $@ $(CC_FLAGS) $< > $(TGT_DIR)/$*.d

clean:
	@echo Removing $(TGT_DIR)
	@rm -rf $(TGT_DIR)
