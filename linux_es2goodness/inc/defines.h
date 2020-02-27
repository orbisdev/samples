#pragma once
/*
  here are the parts
*/
#include  <GLES2/gl2.h>


/// from fileIO.c
unsigned char *orbisFileGetFileContent( const char *filename );
extern size_t _orbisFile_lastopenFile_size;


/// from shader-common.c
GLuint create_vbo(const GLsizeiptr size, const GLvoid* data, const GLenum usage);
GLuint BuildShader(const char *source, GLenum shaderType);
/// build (and dump) from shader source code
GLuint BuildProgram(const char *vShader, const char *fShader);

//GLuint CreateProgramFromBinary(const char *vShader, const char *fShader);


#ifdef _FT_DEMO_
/// from demo-font.c
#include "freetype-gl.h"
int  es2init_text(int width, int height);
void render_text(void);
void es2sample_end(void);


/// from text_ani.c
#include "text_ani.h"
void es2init_text_ani(int width, int height);
void render_text_ext(fx_entry_t *_ani);
void es2update_text_ani(double elapsedTime);
void es2fini_text_ani(void);
#endif


/// from png.c
GLuint load_png_asset_into_texture(const char* relative_path);
#ifndef _FT_DEMO_
  // Clang Extended Vectors
  typedef float vec2 __attribute__((ext_vector_type(2)));
  typedef float vec4 __attribute__((ext_vector_type(4)));
#endif
extern vec2 tex_size; // last loaded png size as (w, h)


/// from sprite.c
void on_GLES2_Init_sprite(int view_w, int view_h);
void on_GLES2_Size_sprite(int view_w, int view_h);
void on_GLES2_Render_sprite(int num);
void on_GLES2_Update_sprite(int frame);
void on_GLES2_Final_sprite(void);


/// for icons.c, sprite.c
#define NUM_OF_TEXTURES  (6)
#define NUM_OF_SPRITES   (6)
