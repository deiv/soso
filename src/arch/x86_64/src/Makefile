
include common.mk

SRCS := $(shell find $(ROOT_DIR) -name '*.c')
OBJS := $(patsubst $(ROOT_DIR)/%.c, $(BUILD_DIR)/arch/x86_64/src/%.o, $(SRCS))
DEPS := $(patsubst $(ROOT_DIR)/%.c, $(BUILD_DIR)/arch/x86_64/src/%.d, $(SRCS))

ASM_SRCS := $(shell find $(ROOT_DIR) -name '*.S')
ASM_OBJS := $(patsubst $(ROOT_DIR)/%.S, $(BUILD_DIR)/arch/x86_64/src/%.o, $(ASM_SRCS))
ASM_DEPS := $(patsubst $(ROOT_DIR)/%.S, $(BUILD_DIR)/arch/x86_64/src/%.d, $(ASM_SRCS))

.PHONY: all
all: $(OBJS) $(ASM_OBJS)

include $(DEPS)
include $(ASM_DEPS)
