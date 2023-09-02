
include common.mk

SUBDIRS=x86_64

.PHONY: all
all: $(SUBDIRS)

.PHONY: x86_64
x86_64:
	$(MAKE) -I $(TOP) -C x86_64/src

