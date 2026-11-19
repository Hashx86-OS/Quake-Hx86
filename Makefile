SUBDIRS = Quake

# libhx86 lives in the Hashx86-OS repository. Point KERNEL_DIR at a checkout
# of that repository; CI sets this to its fetched kernel tree.
KERNEL_DIR ?= ../Hashx86-OS
LIBHX86_DIR = $(KERNEL_DIR)/user_prog/libhx86
LIBHX86_INCLUDE = $(LIBHX86_DIR)/include
LIBMBEDTLS_DIR = $(KERNEL_DIR)/user_prog/libmbedtls
LIBMBEDTLS_INCLUDE = $(LIBMBEDTLS_DIR)/include
MBEDTLS_INCLUDE = $(KERNEL_DIR)/third_party/mbedtls/include
KERNEL_INCLUDE = $(KERNEL_DIR)/include
LIBHX86_LIB_DIR = $(KERNEL_DIR)/build/libhx86
LIBMBEDTLS_LIB_DIR = $(KERNEL_DIR)/build/libmbedtls

.PHONY: all libhx86 libmbedtls check-kernel $(SUBDIRS) clean

all: libhx86 libmbedtls $(SUBDIRS)

check-kernel:
	@if [ ! -d "$(LIBHX86_DIR)" ]; then \
		echo "error: libhx86 not found at $(LIBHX86_DIR)"; \
		echo "Set KERNEL_DIR to a Hashx86-OS checkout."; \
		exit 1; \
	fi
	@if [ ! -d "$(LIBMBEDTLS_DIR)" ] || [ ! -d "$(MBEDTLS_INCLUDE)" ]; then \
		echo "error: mbedTLS sources not found under $(KERNEL_DIR)"; \
		echo "Set KERNEL_DIR to a Hashx86-OS checkout."; \
		exit 1; \
	fi

# Build the kernel libraries first.
libhx86: check-kernel
	@echo ""
	@echo "--- Building libhx86 from $(KERNEL_DIR) ---"
	@$(MAKE) -C "$(LIBHX86_DIR)"

libmbedtls: libhx86
	@echo ""
	@echo "--- Building libmbedtls from $(KERNEL_DIR) ---"
	@$(MAKE) -C "$(LIBMBEDTLS_DIR)"

# Build the apps against those exact libraries.
$(SUBDIRS): libhx86 libmbedtls
	@echo ""
	@echo "--- Building $@ ---"
	@$(MAKE) -C $@ \
		LIBHX86_INCLUDE="$(abspath $(LIBHX86_INCLUDE))" \
		LIBMBEDTLS_INCLUDE="$(abspath $(LIBMBEDTLS_INCLUDE))" \
		MBEDTLS_INCLUDE="$(abspath $(MBEDTLS_INCLUDE))" \
		KERNEL_INCLUDE="$(abspath $(KERNEL_INCLUDE))" \
		LIBHX86_LIB_DIR="$(abspath $(LIBHX86_LIB_DIR))" \
		LIBMBEDTLS_LIB_DIR="$(abspath $(LIBMBEDTLS_LIB_DIR))"

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
