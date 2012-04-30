#
#
#

TOP ?= $(shell pwd)

SRC_DIR=$(TOP)/src
BIN_DIR=$(TOP)/bin
OBJ_DIR=$(TOP)/obj
INCLUDE_DIR=$(TOP)/include

C_SRCS=$(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.c))
ASM_SRCS=$(patsubst $(SRC_DIR)/%.s,$(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.s))
#OBJS=boot.o main.o monitor.o common.o descriptor_tables.o isr.o interrupt.o gdt.o timer.o \
#        kheap.o paging.o ordered_array.o fs.o initrd.o task.o process.o syscall.o

# first object to be linked (it haves the multiboot record)
BOOT_OBJ=$(OBJ_DIR)/boot.o

EXE_NAME=kernel.bin

DEFINES=

FLAGS=$(addprefix -D,$(DEFINES))
CFLAGS=-nostdlib -nostdinc -fno-builtin  -fno-stack-protector -iquote$(INCLUDE_DIR) $(FLAGS) -c
LDFLAGS=-Tlink.ld
ASFLAGS=-felf -I. -I$(INCLUDE_DIR) $(FLAGS)

TARGET=
CC=$(TARGET)-gcc
LD=$(TARGET)-ld

all: compile link

clean:
	rm -Rf $(OBJ_DIR)
	rm -f $(BIN_DIR)/$(EXE_NAME)

compile: $(OBJ_DIR) $(C_SRCS) $(ASM_SRCS)

link:
	$(LD) $(LDFLAGS) -o $(BIN_DIR)/$(EXE_NAME) $(BOOT_OBJ) $(C_SRCS) $(filter-out $(BOOT_OBJ),$(ASM_SRCS))

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

$(C_SRCS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<

$(ASM_SRCS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.s
	nasm $(ASFLAGS) -o $@ $<

image:
	losetup /dev/loop0 "$(BIN_DIR)/floppy.img"
	mount /dev/loop0 /mnt/tmp
	cp "$(BIN_DIR)/$(EXE_NAME)" "/mnt/tmp/$(EXE_NAME)"
	sync
	sync
	sync
	umount /dev/loop0
	losetup -d /dev/loop0

.PHONY : all clean compile link image
