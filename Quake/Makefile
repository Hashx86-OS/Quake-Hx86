GPP = g++
LD = ld

INCLUDES = -Iinclude -Iengine -I../libhx86/include -I.
BASE_CXXFLAGS = -m32 -O2 -g -ffreestanding -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fcommon -fomit-frame-pointer -fpermissive -Wno-write-strings -Wno-return-type $(INCLUDES)
ENGINE_CXXFLAGS = $(BASE_CXXFLAGS) -Dnew=new_block -Did386=0
OUR_CXXFLAGS = $(BASE_CXXFLAGS)
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
	$(ENGINE_SRC_DIR)/snd_null.c

ENGINE_C += \
	$(ENGINE_SRC_DIR)/net_loop.c \
	$(ENGINE_SRC_DIR)/net_none.c \
	$(ENGINE_SRC_DIR)/net_main.c

# All files compiled as C++ (Quake C code is compatible)
ENGINE_CXX = $(ENGINE_C)
OUR_SRC = prog.cpp compat.c sys_hx86.c vid_hx86.c in_hx86.c

ENGINE_OBJS = $(patsubst $(ENGINE_SRC_DIR)/%.c,$(ENGINE_OBJS_DIR)/%.o,$(ENGINE_CXX))
OUR_OBJS = $(addprefix $(ENGINE_OBJS_DIR)/, $(OUR_SRC:.cpp=.o))
OUR_OBJS := $(OUR_OBJS:.c=.o)

ALL_OBJS = $(ENGINE_OBJS) $(OUR_OBJS)
TARGET = $(BUILD_DIR)/Quake.bin

.PHONY: all clean

all: $(TARGET)

$(ENGINE_OBJS_DIR)/%.o: $(ENGINE_SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(GPP) $(ENGINE_CXXFLAGS) -o $@ -c $<

$(ENGINE_OBJS_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(GPP) $(OUR_CXXFLAGS) -o $@ -c $<

$(ENGINE_OBJS_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(GPP) $(OUR_CXXFLAGS) -o $@ -c $<

$(TARGET): $(ALL_OBJS) linker.ld
	@mkdir -p $(BUILD_DIR)
	$(LD) -melf_i386 -T linker.ld -o $@ $(ALL_OBJS) -L../build/libhx86 -lhx86

clean:
	rm -rf $(ENGINE_OBJS_DIR)
	rm -f $(TARGET)
