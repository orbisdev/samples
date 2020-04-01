# make -f pl_mpeg.makefile
# make -f pl_mpeg.makefile clean
# make -f pl_mpeg.makefile && ./egl_demo_pl_mpeg


INC  =-Iinc
# just build libnfs, we need to link_to, and use_it at runtime
INC +=-I/Archive/fuse-work/libnfs-libnfs-4.0.0/include
INC +=-I/Archive/fuse-work/libnfs-libnfs-4.0.0/include/nfsc


EXE := egl_demo_pl_mpeg

#SRC_DIR := $(PS4SDK)/../liborbis/portlibs/libfreetype-gl2/source
OBJ_DIR := obj

SRC := egl.c fileIO.c shader_common.c user_nfs.c pl_mpeg.c timing.c
SRC += $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CC       := clang
CPPFLAGS := 
CFLAGS   := -O3 -msse2
LDLIBS   := -lX11 -lEGL -lGLESv2 -lm -DPL_MPEG -D_LIBAO_ -lao -lnfs -DHAVE_NFS
#provide libnfs.la for the linker:
LDLIBS   += -L/Archive/fuse-work/libnfs-libnfs-4.0.0/build/lib/.libs
#provide libnfs.so for the system:
#sudo ln -s /Archive/fuse-work/libnfs-libnfs-4.0.0/build/lib/.libs/libnfs.so.13.0.0 /lib64/libnfs.so.13


.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(INC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@ 

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(INC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir $@

clean:
	$(RM) $(OBJ_DIR)/*.o $(EXE)
