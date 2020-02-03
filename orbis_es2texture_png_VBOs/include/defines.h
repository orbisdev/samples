#pragma once


#define HAVE_SHACC // main switch: undefined to use embedded .sb


#define NUM_OF_TEXTURES  (6)

#ifdef HAVE_SHACC
    #include "icons_shaders.h"
    #include "sprite_shaders.h"
#else
    #include "icons_binary_shaders.h"
    #include "sprite_binary_shaders.h"
#endif


// Clang Extended Vectors
typedef float vec2 __attribute__((ext_vector_type(2)));
typedef float vec4 __attribute__((ext_vector_type(4)));


// from shader_common.c
GLuint create_vbo(const GLsizeiptr size, const GLvoid* data, const GLenum usage);
#ifdef HAVE_SHACC
GLuint BuildProgram(const char *vShader, const char *fShader);
#else
GLuint CreateProgramFromBinary(unsigned int i) ;
#endif

// from png.c
extern vec2 tex_size; // last loaded png size as (w, h)
GLuint load_png_asset_into_texture(const char* relative_path);

// from timing.c
unsigned int get_time_ms(void);

// from VBOs.c
void on_GLES2_Init(int view_w, int view_h);
void on_GLES2_Size(int view_w, int view_h);
void on_GLES2_Render(int num);
void on_GLES2_Update(int frame);
void on_GLES2_Final(void);
