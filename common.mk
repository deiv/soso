
ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

$(BUILD_DIR)/%.d: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	@set -e
	rm -f $@
	$(CPP) \
		$(KERNEL_INCLUDE_CFLAGS) \
		$(USER_API_INCLUDE_CFLAGS) \
		$(CFLAGS) \
		$(CPPFLAGS) \
		-M $< > $@.tmp
	sed 's,\($(*F)\)\.o[ :]*,$(BUILD_DIR)/$*.o $@ : ,g' < $@.tmp > $@
	rm -f $@.tmp

$(BUILD_DIR)/%.d: $(SRC_DIR)/%.S
	mkdir -p $(dir $@)
	@set -e
	rm -f $@
	$(CPP) \
		$(KERNEL_INCLUDE_ASMFLAGS) \
	  	$(CFLAGS) \
	  	$(CPPFLAGS) \
	  	-D__ASSEMBLY__=1 \
		-M $< > $@.tmp
	sed 's,\($(*F)\)\.o[ :]*,$(BUILD_DIR)/$*.o $@ : ,g' < $@.tmp > $@
	rm -f $@.tmp

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) -c \
		$(KERNEL_INCLUDE_CFLAGS) \
		$(USER_API_INCLUDE_CFLAGS) \
		$(CFLAGS) \
		$< \
		-o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.S
	mkdir -p $(dir $@)
	$(CC) -c \
		$(KERNEL_INCLUDE_ASMFLAGS) \
		$(CFLAGS) \
		-D__ASSEMBLY__=1 \
		$< \
		-o $@
