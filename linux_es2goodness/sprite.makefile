# make -f sprite.makefile
# make -f sprite.makefile clean
# make -f sprite.makefile && ./egl_demo

INC=-Iinc


EXE := egl_demo_sprite

#SRC_DIR := $(PS4SDK)/../liborbis/portlibs/libfreetype-gl2/source
OBJ_DIR := obj

SRC := egl.c fileIO.c shader_common.c png.c sprite.c
SRC += $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CC       := clang
CPPFLAGS := 
CFLAGS   := -Wall -O3
LDLIBS   := -lX11 -lEGL -lGLESv2 -lm -lpng -D_SPRITE_ 

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(INC) $(LDFLAGS) $^ $(LDLIBS) -o $@ 

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(INC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir $@

clean:
	$(RM) $(OBJ_DIR)/*.o $(EXE)
