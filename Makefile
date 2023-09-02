
export TOP ?= $(shell pwd)

#
# dest arch
#
ARCH=x86_64

#
# tooling
#
export CC=$(ARCH)-elf-gcc -D__KERNEL__
export CPP=$(CC) -E
export OBJCOPY=$(ARCH)-elf-objcopy

#
# dirs
#
export BUILD_DIR=$(TOP)/build
export SRC_DIR=$(TOP)/src

DIST_DIR=$(TOP)/dist
x86_64_DIST_DIR=${DIST_DIR}/${ARCH}

TARGET_DIR=$(TOP)/target
x86_64_TARGET_DIR=${TARGET_DIR}/${ARCH}

#
# files
#

LINKER_SCRIPT_IN=${x86_64_TARGET_DIR}/linker.ld.in
LINKER_SCRIPT=${BUILD_DIR}/linker.ld

KERNEL_EXE=kernel.bin
KERNEL_ISO=kernel.iso

#
# asm and C flags
#

export KERNEL_INCLUDE_DIR=$(SRC_DIR)/include
export KERNEL_ARCH_INCLUDE_DIR=$(SRC_DIR)/arch/${ARCH}/include

export USER_API_INCLUDE_DIR=$(SRC_DIR)/userapi/include
export USER_API_ARCH_INCLUDE_DIR=$(SRC_DIR)/arch/${ARCH}/userapi/include

export KERNEL_INCLUDE_CFLAGS=-I $(KERNEL_INCLUDE_DIR) -I $(KERNEL_ARCH_INCLUDE_DIR)
export KERNEL_INCLUDE_ASMFLAGS=-I $(KERNEL_INCLUDE_DIR) -I $(KERNEL_ARCH_INCLUDE_DIR)
export USER_API_INCLUDE_CFLAGS=-I $(USER_API_INCLUDE_DIR) -I $(USER_API_ARCH_INCLUDE_DIR)

export CFLAGS=
CFLAGS += -ffreestanding
CFLAGS += -mcmodel=kernel
CFLAGS += -mno-red-zone
CFLAGS += -nostdlib
CFLAGS += -no-pie
CFLAGS += -Wimplicit-fallthrough
CFLAGS += -g

export CPPFLAGS=

#
# rules
#

.PHONY: all
all: build

.PHONY: all-debug
all: build
	$(OBJCOPY) \
		--only-keep-debug  \
		${x86_64_DIST_DIR}/${KERNEL_EXE} \
		$(BUILD_DIR)/kernel.sym

# TODO: add objs files as prerequisite
.PHONY: build
build: ${x86_64_DIST_DIR} ${LINKER_SCRIPT}
	$(MAKE) -I $(TOP) -C src
	$(CC) -n \
		-T ${LINKER_SCRIPT} \
		-o ${x86_64_DIST_DIR}/${KERNEL_EXE} \
		$(CFLAGS) \
		-nostdlib \
		`find $(BUILD_DIR) -name '*.o'`
	cp ${x86_64_DIST_DIR}/${KERNEL_EXE} ${x86_64_TARGET_DIR}/iso/boot/
	grub-mkrescue \
		/usr/lib/grub/i386-pc \
		-o ${x86_64_DIST_DIR}/${KERNEL_ISO} \
		${x86_64_TARGET_DIR}/iso

${x86_64_DIST_DIR}:
	mkdir -p $@

${LINKER_SCRIPT}: ${LINKER_SCRIPT_IN}
	mkdir -p $(dir $@)
	$(ARCH)-elf-cpp -P -o $@ $<

.PHONY: clean
clean:
	rm -Rf ${BUILD_DIR}
	rm -Rf ${DIST_DIR}
	rm -f ${x86_64_TARGET_DIR}/iso/boot/${KERNEL_EXE}


#
# emulator rules
#
.PHONY: emulator
emulator:
	qemu-system-x86_64 -cdrom ${x86_64_DIST_DIR}/${KERNEL_ISO} -serial stdio
	#-trace events=/tmp/events

.PHONY: emulator-debug
emulator-debug:
	qemu-system-x86_64 -s -S -cdrom ${x86_64_DIST_DIR}/${KERNEL_ISO} -serial stdio
	#-trace events=/tmp/events -cpu "core2duo-v1"
