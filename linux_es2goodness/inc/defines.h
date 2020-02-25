#pragma once

#ifndef _FT_DEMO_
// Clang Extended Vectors
typedef float vec2 __attribute__((ext_vector_type(2)));
typedef float vec4 __attribute__((ext_vector_type(4)));
#endif

/// from png.c
GLuint load_png_asset_into_texture(const char* relative_path);
extern vec2 tex_size; // last loaded png size as (w, h)


/// from shader-common.c
GLuint create_vbo(const GLsizeiptr size, const GLvoid* data, const GLenum usage);
GLuint BuildShader(const char *source, GLenum shaderType);
/// build (and dump) from shader source code
GLuint BuildProgram(const char *vShader, const char *fShader);

//GLuint CreateProgramFromBinary(const char *vShader, const char *fShader);

/// from fileIO.c
unsigned char *orbisFileGetFileContent( const char *filename );
extern size_t _orbisFile_lastopenFile_size;

/// for icons.c, sprite.c
#define NUM_OF_TEXTURES  (6)
#define NUM_OF_SPRITES   (6)
