# make -f text_ani.makefile
# make -f text_ani.makefile clean
# make -f text_ani.makefile && ./egl_demo

INC=-I$(PS4SDK)/include/freetype-gl2 \
	-I$(PS4SDK)/include/freetype2 \
	-Iinc


EXE := egl_demo_text_ani

SRC_DIR := $(PS4SDK)/../liborbis/portlibs/libfreetype-gl2/source
OBJ_DIR := obj

SRC := egl.c fileIO.c shader_common.c demo-font.c text_ani.c
SRC += $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CC       := clang
CPPFLAGS := 
CFLAGS   := -Wall -O3
LDLIBS   := -lX11 -lEGL -lGLESv2 -lm -lfreetype -DFT_DEMO

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
