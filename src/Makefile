
include common.mk

SUBDIRS=arch boot drivers kernel

.PHONY: all
all: $(SUBDIRS)

.PHONY: arch
arch:
	$(MAKE) -I $(TOP) -C arch

.PHONY: boot
boot:
	$(MAKE) -I $(TOP) -C boot

.PHONY: drivers
drivers:
	$(MAKE) -I $(TOP) -C drivers

.PHONY: kernel
kernel:
	$(MAKE) -I $(TOP) -C kernel
