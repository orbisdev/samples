# make -f dr_mp3.makefile
# make -f dr_mp3.makefile clean
# make -f dr_mp3.makefile && ./vt_demo_dr_mp3


INC  =-Iinc
# just build libnfs, we need to link_to, and use_it at runtime
INC +=-I/Archive/fuse-work/libnfs-libnfs-4.0.0/include
INC +=-I/Archive/fuse-work/libnfs-libnfs-4.0.0/include/nfsc


EXE := vt_demo_dr_mp3

#SRC_DIR := $(PS4SDK)/../liborbis/portlibs/libfreetype-gl2/source
OBJ_DIR := obj

SRC := user_nfs.c dr_mp3.c
SRC += $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CC       := clang
CPPFLAGS := 
CFLAGS   := -O3 -msse2
LDLIBS   := -lm -D_LIBAO_ -lao -lnfs -DHAVE_NFS
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
