SUBDIRS = Quake

# libhx86 lives in the Hashx86-OS repository. Point KERNEL_DIR at a checkout
# of that repository; CI sets this to its fetched kernel tree.
KERNEL_DIR ?= ../Hashx86-OS
LIBHX86_DIR = $(KERNEL_DIR)/user_prog/libhx86
LIBHX86_INCLUDE = $(LIBHX86_DIR)/include
KERNEL_INCLUDE = $(KERNEL_DIR)/include
LIBHX86_LIB_DIR = $(KERNEL_DIR)/build/libhx86

.PHONY: all libhx86 check-kernel $(SUBDIRS) clean

all: libhx86 $(SUBDIRS)

check-kernel:
	@if [ ! -d "$(LIBHX86_DIR)" ]; then \
		echo "error: libhx86 not found at $(LIBHX86_DIR)"; \
		echo "Set KERNEL_DIR to a Hashx86-OS checkout."; \
		exit 1; \
	fi

# Build the kernel's libhx86 first.
libhx86: check-kernel
	@echo ""
	@echo "--- Building libhx86 from $(KERNEL_DIR) ---"
	@$(MAKE) -C "$(LIBHX86_DIR)"

# Build the apps against that exact library.
$(SUBDIRS): libhx86
	@echo ""
	@echo "--- Building $@ ---"
	@$(MAKE) -C $@ \
		LIBHX86_INCLUDE="$(abspath $(LIBHX86_INCLUDE))" \
		KERNEL_INCLUDE="$(abspath $(KERNEL_INCLUDE))" \
		LIBHX86_LIB_DIR="$(abspath $(LIBHX86_LIB_DIR))"

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
