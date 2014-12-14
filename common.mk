#
# Common makefile definitions.
#

ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
BUILD_DIR := $(ROOT_DIR)/build

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

LD := ld
LD_BARE := -nostdlib --build-id=none -z max-page-size=0x1
LD_SCRIPT := -T $(ARCH)/kernel.ld
LD_OPTIONS := $(LD_BARE) $(LD_SCRIPT)

ifeq ($(ARCH), x86-64)
	CC_FLAGS += -m64
	AS_FLAGS += -m64
endif
