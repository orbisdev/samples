/*
 * liborbis 
 * Copyright (C) 2015,2016,2017,2018 Antonio Jose Ramos Marquez (aka bigboss) @psxdev on twitter
 * Repository https://github.com/orbisdev/liborbis
 */
#ifndef _ORBISGLTEXTUREMANAGER_H_
#define _ORBISGLTEXTUREMANAGER_H_

#include <GLES2/gl2.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TEXTURE_VERTEX_SHADER "system/shaders/texture1.vertexShader"
#define TEXTURE_FRAGMENT_SHADER "system/shaders/texture1.fragmentShader"

#define ORBISGL_MAX_PROGRAMS 10
#define ORBISGL_MAX_TEXTURES 10

typedef struct OrbisRawTexture
{
	uint32_t *datap;
	unsigned short width;
	unsigned short height;
	unsigned short depth;
}OrbisRawTexture __attribute__ ((aligned (16)));

typedef struct OrbisGlTexture
{
	char name[255];
	GLuint textureId;
	GLuint width;
	GLuint height;	
}OrbisGlTexture;

typedef struct OrbisGlProgram
{
	char name[255];
	GLuint programId;
}OrbisGlProgram;

typedef struct OrbisGlVertexInfo
{
	float x;
	float y;
	float u;
	float v;
}OrbisGlVertexInfo;

typedef struct OrbisGlTextureState
{
	uint32_t screenWidth, screenHeight;
	OrbisGlTexture *texture;
	GLuint bufferId;
	OrbisGlProgram *program;
	GLuint s_xyz_loc;
	GLuint s_uv_loc;
	GLuint s_sampler_loc;
	OrbisGlVertexInfo* bufferData;
	uint32_t count;
}OrbisGlTextureState;

GLuint orbisGlGetProgramId(OrbisGlProgram *program);
OrbisGlProgram *orbisGlGetProgram(char *name);
GLuint orbisGlGetTextureId(OrbisGlTexture* text);
OrbisGlTexture * orbisGlGetTexture(char* name);
OrbisGlTextureState * orbisGlInitTexture(char *name,char *path,OrbisGlProgram *program,int x, int y);
void orbisGlFinishTexture(OrbisGlTextureState *p);
void orbisGlTextureAdd(OrbisGlTextureState *p, int x, int y,int w,int h);
void orbisGlDrawTexture(OrbisGlTextureState *p);

OrbisGlTexture * orbistGlTextureManagerRegisterTextureFromBuffer(char* name,int width,int height,int format,void *buf);
OrbisGlTexture * orbisGlTextureManagerRegisterPngTexture(char* name, char* path);
OrbisGlProgram * orbisGlTextureManagerInit();

#ifdef __cplusplus
}
#endif

#endif