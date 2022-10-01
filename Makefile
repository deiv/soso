TOP ?= $(shell pwd)

x86_64_ARCH=x86_64

CFLAGS=-ffreestanding -mcmodel=kernel -mno-red-zone -nostdlib -no-pie

BUILD_DIR=$(TOP)/build

DIST_DIR=$(TOP)/dist
x86_64_DIST_DIR=${DIST_DIR}/${x86_64_ARCH}

TARGET_DIR=$(TOP)/target
x86_64_TARGET_DIR=${TARGET_DIR}/${x86_64_ARCH}

LINKER_SCRIPT_IN=${x86_64_TARGET_DIR}/linker.ld.in
LINKER_SCRIPT=${BUILD_DIR}/linker.ld

KERNEL_EXE=kernel.bin
KERNEL_ISO=kernel.iso



kernel_source_files := $(shell find src/kernel -name *.c)
kernel_object_files := $(patsubst src/kernel/%.c, ${BUILD_DIR}/kernel/%.o, $(kernel_source_files))

boot_source_files := $(shell find src/boot -name *.c)
boot_object_files := $(patsubst src/boot/%.c, ${BUILD_DIR}/boot/%.o, $(boot_source_files))

x86_64_c_source_files := $(shell find src/arch/x86_64 -name *.c)
x86_64_c_object_files := $(patsubst src/arch/x86_64/%.c, ${BUILD_DIR}/x86_64/%.o, $(x86_64_c_source_files))

x86_64_asm_source_files := $(shell find src/arch/x86_64 -name *.S)
x86_64_asm_object_files := $(patsubst src/arch/x86_64/%.S, ${BUILD_DIR}/x86_64/%.o, $(x86_64_asm_source_files))

x86_64_object_files := $(x86_64_c_object_files) $(x86_64_asm_object_files)

$(kernel_object_files): ${BUILD_DIR}/kernel/%.o : src/kernel/%.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/include $(CFLAGS) $(patsubst ${BUILD_DIR}/kernel/%.o, src/kernel/%.c, $@) -o $@

$(boot_object_files): ${BUILD_DIR}/boot/%.o : src/boot/%.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/include $(CFLAGS)  $(patsubst ${BUILD_DIR}/boot/%.o, src/boot/%.c, $@) -o $@

$(x86_64_c_object_files): ${BUILD_DIR}/x86_64/%.o : src/arch/x86_64/%.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/include -I src/arch/x86_64/include $(CFLAGS)  $(patsubst ${BUILD_DIR}/x86_64/%.o, src/arch/x86_64/%.c, $@) -o $@

$(x86_64_asm_object_files): ${BUILD_DIR}/x86_64/%.o : src/arch/x86_64/%.S
	mkdir -p $(dir $@)
	#nasm -f elf64 $(patsubst ${BUILD_DIR}/x86_64/%.o, src/arch/x86_64/%.asm, $@) -o $@
	x86_64-elf-gcc  -c $(CFLAGS) -I src/arch/x86_64/include $(patsubst ${BUILD_DIR}/x86_64/%.o, src/arch/x86_64/%.S, $@) -o $@

${x86_64_DIST_DIR}:
	mkdir -p $@

${LINKER_SCRIPT}: ${LINKER_SCRIPT_IN}
	mkdir -p $(dir $@)
	x86_64-elf-cpp -P -o $@ $<

.PHONY: build-x86_64
build-x86_64: ${x86_64_DIST_DIR} ${LINKER_SCRIPT} $(kernel_object_files) $(boot_object_files) $(x86_64_object_files)
	#x86_64-elf-ld  -n  -o ${x86_64_DIST_DIR}/${KERNEL_EXE} -T ${LINKER_SCRIPT} $(kernel_object_files) $(boot_object_files) $(x86_64_object_files)
	x86_64-elf-gcc -n -T ${LINKER_SCRIPT} -o ${x86_64_DIST_DIR}/${KERNEL_EXE} $(CFLAGS) -O2 -nostdlib $(kernel_object_files) $(boot_object_files) $(x86_64_object_files)
	cp ${x86_64_DIST_DIR}/${KERNEL_EXE} ${x86_64_TARGET_DIR}/iso/boot/
	grub-mkrescue /usr/lib/grub/i386-pc -o ${x86_64_DIST_DIR}/${KERNEL_ISO} ${x86_64_TARGET_DIR}/iso

.PHONY: clean
clean:
	rm -Rf ${BUILD_DIR}
	rm -Rf ${DIST_DIR}

.PHONY: default
default: build-x86_64
