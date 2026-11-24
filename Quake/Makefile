GPP = g++
GCC = gcc
LD = ld

LIBHX86_INCLUDE ?= ../Hashx86-OS/user_prog/libhx86/include
LIBMBEDTLS_INCLUDE ?= ../Hashx86-OS/user_prog/libmbedtls/include
MBEDTLS_INCLUDE ?= ../Hashx86-OS/third_party/mbedtls/include
KERNEL_INCLUDE ?= ../Hashx86-OS/include
LIBHX86_LIB_DIR ?= ../Hashx86-OS/build/libhx86
LIBMBEDTLS_LIB_DIR ?= ../Hashx86-OS/build/libmbedtls

INCLUDES = -Iinclude -Iengine -I$(LIBHX86_INCLUDE) -I$(LIBMBEDTLS_INCLUDE) \
           -I$(MBEDTLS_INCLUDE) -I$(KERNEL_INCLUDE) -I.
BASE_CXXFLAGS = -m32 -O2 -g -ffreestanding -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fcommon -fomit-frame-pointer -fpermissive -Wno-write-strings -Wno-return-type $(INCLUDES)
ENGINE_CXXFLAGS = $(BASE_CXXFLAGS) -Dnew=new_block -Did386=0
OUR_CXXFLAGS = $(BASE_CXXFLAGS)

ARCHIVE_INCLUDES = -Idata/compat -Iinclude -I$(LIBHX86_INCLUDE) -I$(KERNEL_INCLUDE) \
                   -I../third_party/miniz -I../third_party/lhasa/lib/public \
                   -I../third_party/lhasa/lib
ARCHIVE_CFLAGS = -m32 -O2 -g -ffreestanding -nostdlib -fno-builtin -fno-common \
                 -fno-omit-frame-pointer -MMD -MP \
                 -DMINIZ_NO_STDIO -DMINIZ_NO_DEFLATE_APIS -DMINIZ_NO_TIME -DNDEBUG \
                 $(ARCHIVE_INCLUDES)

ENGINE_SRC_DIR = engine
ENGINE_OBJS_DIR = ../build/obj/quake
BUILD_DIR = ../build

ENGINE_C = \
	$(ENGINE_SRC_DIR)/chase.c \
	$(ENGINE_SRC_DIR)/cl_demo.c \
	$(ENGINE_SRC_DIR)/cl_input.c \
	$(ENGINE_SRC_DIR)/cl_main.c \
	$(ENGINE_SRC_DIR)/cl_parse.c \
	$(ENGINE_SRC_DIR)/cl_tent.c \
	$(ENGINE_SRC_DIR)/cmd.c \
	$(ENGINE_SRC_DIR)/common.c \
	$(ENGINE_SRC_DIR)/console.c \
	$(ENGINE_SRC_DIR)/crc.c \
	$(ENGINE_SRC_DIR)/cvar.c \
	$(ENGINE_SRC_DIR)/d_edge.c \
	$(ENGINE_SRC_DIR)/d_fill.c \
	$(ENGINE_SRC_DIR)/d_init.c \
	$(ENGINE_SRC_DIR)/d_modech.c \
	$(ENGINE_SRC_DIR)/d_part.c \
	$(ENGINE_SRC_DIR)/d_polyse.c \
	$(ENGINE_SRC_DIR)/d_scan.c \
	$(ENGINE_SRC_DIR)/d_sky.c \
	$(ENGINE_SRC_DIR)/d_sprite.c \
	$(ENGINE_SRC_DIR)/d_surf.c \
	$(ENGINE_SRC_DIR)/d_vars.c \
	$(ENGINE_SRC_DIR)/d_zpoint.c \
	$(ENGINE_SRC_DIR)/draw.c \
	$(ENGINE_SRC_DIR)/host.c \
	$(ENGINE_SRC_DIR)/host_cmd.c \
	$(ENGINE_SRC_DIR)/keys.c \
	$(ENGINE_SRC_DIR)/mathlib.c \
	$(ENGINE_SRC_DIR)/menu.c \
	$(ENGINE_SRC_DIR)/model.c \
	$(ENGINE_SRC_DIR)/nonintel.c \
	$(ENGINE_SRC_DIR)/pr_cmds.c \
	$(ENGINE_SRC_DIR)/pr_edict.c \
	$(ENGINE_SRC_DIR)/pr_exec.c \
	$(ENGINE_SRC_DIR)/r_aclip.c \
	$(ENGINE_SRC_DIR)/r_alias.c \
	$(ENGINE_SRC_DIR)/r_bsp.c \
	$(ENGINE_SRC_DIR)/r_draw.c \
	$(ENGINE_SRC_DIR)/r_edge.c \
	$(ENGINE_SRC_DIR)/r_efrag.c \
	$(ENGINE_SRC_DIR)/r_light.c \
	$(ENGINE_SRC_DIR)/r_main.c \
	$(ENGINE_SRC_DIR)/r_misc.c \
	$(ENGINE_SRC_DIR)/r_part.c \
	$(ENGINE_SRC_DIR)/r_sky.c \
	$(ENGINE_SRC_DIR)/r_sprite.c \
	$(ENGINE_SRC_DIR)/r_surf.c \
	$(ENGINE_SRC_DIR)/r_vars.c \
	$(ENGINE_SRC_DIR)/sbar.c \
	$(ENGINE_SRC_DIR)/screen.c \
	$(ENGINE_SRC_DIR)/sv_main.c \
	$(ENGINE_SRC_DIR)/sv_move.c \
	$(ENGINE_SRC_DIR)/sv_phys.c \
	$(ENGINE_SRC_DIR)/sv_user.c \
	$(ENGINE_SRC_DIR)/view.c \
	$(ENGINE_SRC_DIR)/wad.c \
	$(ENGINE_SRC_DIR)/world.c \
	$(ENGINE_SRC_DIR)/zone.c

ENGINE_C += \
	$(ENGINE_SRC_DIR)/cd_null.c \
	$(ENGINE_SRC_DIR)/snd_dma.c \
	$(ENGINE_SRC_DIR)/snd_mem.c \
	$(ENGINE_SRC_DIR)/snd_mix.c

ENGINE_C += \
	$(ENGINE_SRC_DIR)/net_loop.c \
	$(ENGINE_SRC_DIR)/net_none.c \
	$(ENGINE_SRC_DIR)/net_main.c

# All files compiled as C++ (Quake C code is compatible)
ENGINE_CXX = $(ENGINE_C)
OUR_SRC = prog.cpp data/Archive.cpp data/ArchivePort.cpp data/ArchiveCompat.cpp \
          data/HttpClient.cpp data/StoreUtil.cpp compat.c sys_hx86.c vid_hx86.c in_hx86.c \
          snd_hx86.c

# Vendored archive readers. These stay C so their upstream code is unchanged.
ARCHIVE_C = \
	../third_party/miniz/miniz.c \
	../third_party/miniz/miniz_tinfl.c \
	../third_party/miniz/miniz_zip.c \
	../third_party/lhasa/lib/crc16.c \
	../third_party/lhasa/lib/ext_header.c \
	../third_party/lhasa/lib/lha_basic_reader.c \
	../third_party/lhasa/lib/lha_decoder.c \
	../third_party/lhasa/lib/lha_endian.c \
	../third_party/lhasa/lib/lha_file_header.c \
	../third_party/lhasa/lib/lha_input_stream.c \
	../third_party/lhasa/lib/lha_reader.c \
	../third_party/lhasa/lib/lh1_decoder.c \
	../third_party/lhasa/lib/lh5_decoder.c \
	../third_party/lhasa/lib/lh6_decoder.c \
	../third_party/lhasa/lib/lh7_decoder.c \
	../third_party/lhasa/lib/lhx_decoder.c \
	../third_party/lhasa/lib/lk7_decoder.c \
	../third_party/lhasa/lib/lz5_decoder.c \
	../third_party/lhasa/lib/lzs_decoder.c \
	../third_party/lhasa/lib/macbinary.c \
	../third_party/lhasa/lib/null_decoder.c \
	../third_party/lhasa/lib/pm1_decoder.c \
	../third_party/lhasa/lib/pm2_decoder.c

ENGINE_OBJS = $(patsubst $(ENGINE_SRC_DIR)/%.c,$(ENGINE_OBJS_DIR)/%.o,$(ENGINE_CXX))
OUR_OBJS = $(addprefix $(ENGINE_OBJS_DIR)/, $(OUR_SRC:.cpp=.o))
OUR_OBJS := $(OUR_OBJS:.c=.o)
ARCHIVE_OBJS = $(addprefix $(ENGINE_OBJS_DIR)/archive/, $(notdir $(ARCHIVE_C:.c=.o)))

ALL_OBJS = $(ENGINE_OBJS) $(OUR_OBJS) $(ARCHIVE_OBJS)
TARGET = $(BUILD_DIR)/Quake.bin

.PHONY: all clean

all: $(TARGET)

$(ENGINE_OBJS_DIR)/%.o: $(ENGINE_SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(GPP) $(ENGINE_CXXFLAGS) -o $@ -c $<

$(ENGINE_OBJS_DIR)/data/%.o: data/%.cpp
	@mkdir -p $(dir $@)
	$(GPP) $(OUR_CXXFLAGS) -Idata/compat -I../third_party/miniz \
		-I../third_party/lhasa/lib/public -I../third_party/lhasa/lib -o $@ -c $<

$(ENGINE_OBJS_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(GPP) $(OUR_CXXFLAGS) -o $@ -c $<

$(ENGINE_OBJS_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(GPP) $(OUR_CXXFLAGS) -o $@ -c $<

$(ENGINE_OBJS_DIR)/archive/%.o: ../third_party/miniz/%.c
	@mkdir -p $(dir $@)
	$(GCC) $(ARCHIVE_CFLAGS) -o $@ -c $<

$(ENGINE_OBJS_DIR)/archive/%.o: ../third_party/lhasa/lib/%.c
	@mkdir -p $(dir $@)
	$(GCC) $(ARCHIVE_CFLAGS) -o $@ -c $<

$(TARGET): $(ALL_OBJS) linker.ld
	@mkdir -p $(BUILD_DIR)
	$(LD) -melf_i386 -T linker.ld -o $@ $(ALL_OBJS) \
		-L$(LIBHX86_LIB_DIR) -L$(LIBMBEDTLS_LIB_DIR) -lmbedtls -lhx86

-include $(ALL_OBJS:.o=.d)

clean:
	rm -rf $(ENGINE_OBJS_DIR)
	rm -f $(TARGET)
