TOP ?= $(shell pwd)

x86_64_ARCH=x86_64

DIST_DIR=$(TOP)/dist
x86_64_DIST_DIR=${DIST_DIR}/${x86_64_ARCH}

TARGET_DIR=$(TOP)/target
x86_64_TARGET_DIR=${TARGET_DIR}/${x86_64_ARCH}

KERNEL_EXE=kernel.bin
KERNEL_ISO=kernel.iso



kernel_source_files := $(shell find src/kernel -name *.c)
kernel_object_files := $(patsubst src/kernel/%.c, build/kernel/%.o, $(kernel_source_files))

x86_64_c_source_files := $(shell find src/arch/x86_64 -name *.c)
x86_64_c_object_files := $(patsubst src/arch/x86_64/%.c, build/x86_64/%.o, $(x86_64_c_source_files))

x86_64_asm_source_files := $(shell find src/arch/x86_64 -name *.asm)
x86_64_asm_object_files := $(patsubst src/arch/x86_64/%.asm, build/x86_64/%.o, $(x86_64_asm_source_files))

x86_64_object_files := $(x86_64_c_object_files) $(x86_64_asm_object_files)

$(kernel_object_files): build/kernel/%.o : src/kernel/%.c
	mkdir -p $(dir $@) && \
	x86_64-elf-gcc -c -I src/include -ffreestanding $(patsubst build/kernel/%.o, src/kernel/%.c, $@) -o $@

$(x86_64_c_object_files): build/x86_64/%.o : src/arch/x86_64/%.c
	mkdir -p $(dir $@) && \
	x86_64-elf-gcc -c -I src/include -ffreestanding $(patsubst build/x86_64/%.o, src/arch/x86_64/%.c, $@) -o $@

$(x86_64_asm_object_files): build/x86_64/%.o : src/arch/x86_64/%.asm
	mkdir -p $(dir $@) && \
	nasm -f elf64 $(patsubst build/x86_64/%.o, src/arch/x86_64/%.asm, $@) -o $@

${x86_64_DIST_DIR}:
	mkdir -p $@

.PHONY: build-x86_64
build-x86_64: ${x86_64_DIST_DIR} $(kernel_object_files) $(x86_64_object_files)
	x86_64-elf-ld -n -o ${x86_64_DIST_DIR}/${KERNEL_EXE} -T ${x86_64_TARGET_DIR}/linker.ld $(kernel_object_files) $(x86_64_object_files)
	cp ${x86_64_DIST_DIR}/${KERNEL_EXE} ${x86_64_TARGET_DIR}/iso/boot/
	grub-mkrescue /usr/lib/grub/i386-pc -o ${x86_64_DIST_DIR}/${KERNEL_ISO} ${x86_64_TARGET_DIR}/iso

.PHONY: clean
clean:
	rm -Rf build/