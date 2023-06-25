
TOP ?= $(shell pwd)

#
# dest arch
#

x86_64_ARCH=x86_64

#
# dirs
#

BUILD_DIR=$(TOP)/build
SRC_DIR=$(TOP)/src

DIST_DIR=$(TOP)/dist
x86_64_DIST_DIR=${DIST_DIR}/${x86_64_ARCH}

TARGET_DIR=$(TOP)/target
x86_64_TARGET_DIR=${TARGET_DIR}/${x86_64_ARCH}

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

KERNEL_INCLUDE_DIR=$(SRC_DIR)/include
KERNEL_ARCH_INCLUDE_DIR=$(SRC_DIR)/arch/${x86_64_ARCH}/include

USER_API_INCLUDE_DIR=$(SRC_DIR)/userapi/include
USER_API_ARCH_INCLUDE_DIR=$(SRC_DIR)/arch/${x86_64_ARCH}/userapi/include

KERNEL_INCLUDE_CFLAGS=-I $(KERNEL_INCLUDE_DIR) -I $(KERNEL_ARCH_INCLUDE_DIR)
KERNEL_INCLUDE_ASMFLAGS=-I $(KERNEL_ARCH_INCLUDE_DIR)
USER_API_INCLUDE_CFLAGS=-I $(USER_API_INCLUDE_DIR) -I $(USER_API_ARCH_INCLUDE_DIR)

CFLAGS=-ffreestanding -mcmodel=kernel -mno-red-zone -nostdlib -no-pie -Wimplicit-fallthrough

#
# compilation units (very rudimentary)
#

kernel_source_files := $(shell find src/kernel -name *.c)
kernel_object_files := $(patsubst src/kernel/%.c, ${BUILD_DIR}/kernel/%.o, $(kernel_source_files))

boot_source_files := $(shell find src/boot -name *.c)
boot_object_files := $(patsubst src/boot/%.c, ${BUILD_DIR}/boot/%.o, $(boot_source_files))

x86_64_c_source_files := $(shell find src/arch/$(x86_64_ARCH) -name *.c)
x86_64_c_object_files := $(patsubst src/arch/$(x86_64_ARCH)/%.c, ${BUILD_DIR}/$(x86_64_ARCH)/%.o, $(x86_64_c_source_files))

x86_64_asm_source_files := $(shell find src/arch/$(x86_64_ARCH) -name *.S)
x86_64_asm_object_files := $(patsubst src/arch/$(x86_64_ARCH)/%.S, ${BUILD_DIR}/$(x86_64_ARCH)/%.o, $(x86_64_asm_source_files))

x86_64_object_files := $(x86_64_c_object_files) $(x86_64_asm_object_files)

drivers_source_files := $(shell find src/drivers -name *.c)
drivers_object_files := $(patsubst src/drivers/%.c, ${BUILD_DIR}/drivers/%.o, $(drivers_source_files))

#
# rules
#

$(kernel_object_files): ${BUILD_DIR}/kernel/%.o : src/kernel/%.c
	mkdir -p $(dir $@)
	$(x86_64_ARCH)-elf-gcc -c \
		$(KERNEL_INCLUDE_CFLAGS) \
		$(USER_API_INCLUDE_CFLAGS) \
		$(CFLAGS) \
		$(patsubst ${BUILD_DIR}/kernel/%.o, src/kernel/%.c, $@)\
		-o $@

$(boot_object_files): ${BUILD_DIR}/boot/%.o : src/boot/%.c
	mkdir -p $(dir $@)
	$(x86_64_ARCH)-elf-gcc -c \
		$(KERNEL_INCLUDE_CFLAGS) \
		$(CFLAGS) \
		$(patsubst ${BUILD_DIR}/boot/%.o, src/boot/%.c, $@) \
		-o $@

$(x86_64_c_object_files): ${BUILD_DIR}/$(x86_64_ARCH)/%.o : src/arch/$(x86_64_ARCH)/%.c
	mkdir -p $(dir $@)
	$(x86_64_ARCH)-elf-gcc -c \
		$(KERNEL_INCLUDE_CFLAGS) \
		$(USER_API_INCLUDE_CFLAGS) \
		$(CFLAGS) \
		$(patsubst ${BUILD_DIR}/$(x86_64_ARCH)/%.o, src/arch/$(x86_64_ARCH)/%.c, $@) \
		-o $@

$(x86_64_asm_object_files): ${BUILD_DIR}/$(x86_64_ARCH)/%.o : src/arch/$(x86_64_ARCH)/%.S
	mkdir -p $(dir $@)
	$(x86_64_ARCH)-elf-gcc -c \
		$(KERNEL_INCLUDE_ASMFLAGS) \
		$(CFLAGS) \
		$(patsubst ${BUILD_DIR}/$(x86_64_ARCH)/%.o, src/arch/$(x86_64_ARCH)/%.S, $@) \
		-o $@

$(drivers_object_files): ${BUILD_DIR}/drivers/%.o : src/drivers/%.c
	mkdir -p $(dir $@)
	$(x86_64_ARCH)-elf-gcc -c \
		$(KERNEL_INCLUDE_CFLAGS) \
		$(USER_API_INCLUDE_CFLAGS) \
		$(CFLAGS) \
		$(patsubst ${BUILD_DIR}/drivers/%.o, src/drivers/%.c, $@) \
		-o $@

${x86_64_DIST_DIR}:
	mkdir -p $@

${LINKER_SCRIPT}: ${LINKER_SCRIPT_IN}
	mkdir -p $(dir $@)
	$(x86_64_ARCH)-elf-cpp -P -o $@ $<

.PHONY: build-x86_64
build-x86_64: ${x86_64_DIST_DIR} ${LINKER_SCRIPT} $(kernel_object_files) $(boot_object_files) $(x86_64_object_files) $(drivers_object_files)
	#x86_64-elf-ld  -n  -o ${x86_64_DIST_DIR}/${KERNEL_EXE} -T ${LINKER_SCRIPT} $(kernel_object_files) $(boot_object_files) $(x86_64_object_files)
	$(x86_64_ARCH)-elf-gcc -n \
		-T ${LINKER_SCRIPT} \
		-o ${x86_64_DIST_DIR}/${KERNEL_EXE} \
		$(CFLAGS) \
		-O2 \
		-nostdlib \
		$(kernel_object_files) \
		$(boot_object_files) \
		$(x86_64_object_files) \
		$(drivers_object_files)
	cp ${x86_64_DIST_DIR}/${KERNEL_EXE} ${x86_64_TARGET_DIR}/iso/boot/
	grub-mkrescue /usr/lib/grub/i386-pc -o ${x86_64_DIST_DIR}/${KERNEL_ISO} ${x86_64_TARGET_DIR}/iso

.PHONY: clean
clean:
	rm -Rf ${BUILD_DIR}
	rm -Rf ${DIST_DIR}
	rm -f ${x86_64_TARGET_DIR}/iso/boot/${KERNEL_EXE}

.PHONY: default
default: build-x86_64
