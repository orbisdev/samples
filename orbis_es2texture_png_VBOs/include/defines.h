#pragma once


#define  NUM_OF_TEXTURES  (6)


// Clang Extended Vectors
typedef float vec2 __attribute__((ext_vector_type(2)));
typedef float vec4 __attribute__((ext_vector_type(4)));

// from png.c
GLuint create_vbo(const GLsizeiptr size, const GLvoid* data, const GLenum usage);
GLuint load_png_asset_into_texture(const char* relative_path);

// from VBOs.c
void on_GLES2_Init(int view_w, int view_h);
void on_GLES2_Size(int view_w, int view_h);
void on_GLES2_Render(int num);
void on_GLES2_Update(float timeStep_sec);
void on_GLES2_Final(void);
