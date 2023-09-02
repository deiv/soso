
include common.mk

SRCS := $(shell find $(ROOT_DIR) -name '*.c')
OBJS := $(patsubst $(ROOT_DIR)/%.c, $(BUILD_DIR)/boot/%.o, $(SRCS))
DEPS := $(patsubst $(ROOT_DIR)/%.c, $(BUILD_DIR)/boot/%.d, $(SRCS))

.PHONY: all
all: $(OBJS)

include $(DEPS)
