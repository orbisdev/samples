/*
 * liborbis 
 * Copyright (C) 2015,2016,2017,2018 Antonio Jose Ramos Marquez (aka bigboss) @psxdev on twitter
 * Repository https://github.com/orbisdev/liborbis
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kernel.h>
#include <stdbool.h>
#include <sys/fcntl.h>
#include <orbisGl.h>
#include <orbisNfs.h>
#include <debugnet.h>
#include <png.h>  

#include "orbisGlTextureManager.h"

#define PNG_SIGSIZE (8)

int orbisGlTextureCounter=0;
OrbisGlTexture orbisGlTextures[10];

int orbisGlProgramCounter=0;
OrbisGlProgram orbisGlPrograms[10];

GLuint s_xyz_loc;
GLuint s_uv_loc;
GLuint s_sampler_loc;

GLuint orbisGlGetProgramId(OrbisGlProgram *po)
{
	debugNetPrintf(0,"[ORBISGL] %s \n",__FUNCTION__);
	if(po!=NULL)
	{
		return po->programId;
	}
	debugNetPrintf(ERROR,"[ORBISGL] %s program is NULL\n",__FUNCTION__);
	return 0;
}

GLuint orbisGlGetTextureId(OrbisGlTexture *text)
{
	debugNetPrintf(0,"[ORBISGL] %s \n",__FUNCTION__);
	if(text!=NULL)
	{
		return text->textureId;
	}
	debugNetPrintf(ERROR,"[ORBISGL] %s texture is NULL\n",__FUNCTION__);
	return 0;
}

OrbisGlProgram * orbisGlGetProgram(char *name)
{
	debugNetPrintf(0,"[ORBISGL] %s \n",__FUNCTION__);
	if(name==NULL || strlen(name)>255)
	{
		return NULL;
	}
	int i;
	for(i=0;i<orbisGlProgramCounter;i++)
	{
		if(strcmp(name,orbisGlPrograms[i].name)==0)
		{
			return &orbisGlPrograms[i];
		}
	}
	return NULL;
}

OrbisGlTexture * orbisGlGetTexture(char *name)
{
	debugNetPrintf(0,"[ORBISGL] %s \n",__FUNCTION__);
	if(name==NULL || strlen(name)>255)
	{
		return NULL;
	}
	int i;
	for(i=0;i<orbisGlTextureCounter;i++)
	{
		if(strcmp(name,orbisGlTextures[i].name)==0)
		{
			return &orbisGlTextures[i];
		}
	}
	return NULL;
}
OrbisRawTexture * orbisGlCreateEmptyRawTexture(unsigned int w, unsigned int h)
{
	debugNetPrintf(DEBUG,"[ORBISGL] %s \n",__FUNCTION__);

	OrbisRawTexture *img=NULL;
	img=malloc(sizeof(OrbisRawTexture));
	if(img!=NULL)
	{
		img->datap=mmap(NULL,w*h*4,0x01|0x02,0x1000|0x0002,-1,0);
		if(img->datap==NULL)
		{
			free(img);
			return NULL;
		}
		img->width=w;
		img->height=h;
		img->depth=32;
	}
	return img;
}
void orbisGlDestroyRawTexture(OrbisRawTexture *texture)
{
	debugNetPrintf(DEBUG,"[ORBISGL] %s \n",__FUNCTION__);

	if(texture!=NULL)
	{
		if(texture->datap!=NULL)
		{
			munmap(texture->datap,texture->width*texture->height*4);
			texture->datap=NULL;
		}
		free(texture);
	}
}

uint32_t * orbisGlRawTextureGetDataPointer(OrbisRawTexture *texture)
{
	debugNetPrintf(DEBUG,"[ORBISGL] %s \n",__FUNCTION__);

	if(texture)
	{
		return texture->datap;
	}
	return NULL;
}

uint32_t orbisGlRawTextureGetStride(OrbisRawTexture *texture)
{
	debugNetPrintf(DEBUG,"[ORBISGL] %s \n",__FUNCTION__);

	if(texture)
	{
		return texture->width;
	}
	return 0;
}
static void orbisGlReadPngFromBuffer(png_structp png_ptr, png_bytep data, png_size_t length)
{
	debugNetPrintf(DEBUG,"[ORBISGL] %s \n",__FUNCTION__);

	uint64_t *address = png_get_io_ptr(png_ptr);
	memcpy(data, (void *)*address, length);
	*address += length;
}
static OrbisRawTexture *orbisGlLoadPngGeneric(const void *io_ptr,png_rw_ptr read_data_fn)
{
	debugNetPrintf(DEBUG,"[ORBISGL] %s \n",__FUNCTION__);

	png_structp png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
	if (png_ptr==NULL)
	{
		goto error_create_read;
	}

	png_infop info_ptr=png_create_info_struct(png_ptr);
	if (info_ptr==NULL)
	{
		goto error_create_info;
	}

	png_bytep *row_ptrs=NULL;

	if (setjmp(png_jmpbuf(png_ptr))) 
	{
		png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)0);
		if (row_ptrs!=NULL)
		{
			free(row_ptrs);
		}
		return NULL;
	}

	png_set_read_fn(png_ptr,(png_voidp)io_ptr,read_data_fn);
	png_set_sig_bytes(png_ptr,PNG_SIGSIZE);
	png_read_info(png_ptr,info_ptr);

	unsigned int width, height;
	int bit_depth, color_type;

	png_get_IHDR(png_ptr,info_ptr,&width,&height,&bit_depth,&color_type,NULL,NULL,NULL);

	if ((color_type==PNG_COLOR_TYPE_PALETTE && bit_depth<=8)
		|| (color_type==PNG_COLOR_TYPE_GRAY && bit_depth<8)
		|| png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS)
		|| (bit_depth==16))
	{
		png_set_expand(png_ptr);
	}

	if (bit_depth == 16)
		png_set_scale_16(png_ptr);

	if (bit_depth==8 && color_type==PNG_COLOR_TYPE_RGB)
		png_set_filler(png_ptr,0xFF,PNG_FILLER_AFTER);

	if (color_type==PNG_COLOR_TYPE_GRAY ||
	    color_type==PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);

	if (color_type==PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(png_ptr);
		png_set_filler(png_ptr,0xFF,PNG_FILLER_AFTER);
	}

	if (color_type==PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(png_ptr);

	if (png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_ptr);

	if (bit_depth<8)
		png_set_packing(png_ptr);

	png_read_update_info(png_ptr, info_ptr);

	row_ptrs = (png_bytep *)malloc(sizeof(png_bytep)*height);
	if (!row_ptrs)
		goto error_alloc_rows;

	OrbisRawTexture *texture = orbisGlCreateEmptyRawTexture(width,height);
	if (!texture)
		goto error_create_tex;

	uint32_t *texture_data=orbisGlRawTextureGetDataPointer(texture);
	unsigned int stride=orbisGlRawTextureGetStride(texture);

	int i;
	for (i=0;i<height;i++)
	{
		row_ptrs[i]=(png_bytep)(texture_data+i*stride);
	}

	png_read_image(png_ptr, row_ptrs);

	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)0);
	free(row_ptrs);

	return texture;

error_create_tex:
	free(row_ptrs);
error_alloc_rows:
	png_destroy_info_struct(png_ptr,&info_ptr);
error_create_info:
	png_destroy_read_struct(&png_ptr,(png_infopp)0,(png_infopp)0);
error_create_read:
	return NULL;
}

OrbisRawTexture * orbisGlLoadPngFromNfs(const char *path)
{
	debugNetPrintf(DEBUG,"[ORBISGL] %s \n",__FUNCTION__);
	char *buf=orbisNfsGetFileContent(path);
	if(buf)
	{
		if(png_sig_cmp((png_byte *)buf,0,PNG_SIGSIZE)!=0) 
		{
			return NULL;
		}
		uint64_t buffer_address=(uint64_t)buf+PNG_SIGSIZE;
		return orbisGlLoadPngGeneric((void *)&buffer_address,orbisGlReadPngFromBuffer);
	}
	return NULL;
}

OrbisGlTexture * orbisGlTextureManagerRegisterTextureFromBuffer(char* name,int width,int height,int format,void *buf)
{
	debugNetPrintf(DEBUG,"[ORBISGL] %s \n",__FUNCTION__);
	GLuint textureId;
	if(!buf || !name)
	{
		return NULL;
	}
	textureId=orbisGlCreateTexture(width,height,format,buf);
	if(textureId<=0)
	{
		debugNetPrintf(ERROR,"[ORBISGL] %s Texture can't be loaded %s, skip 2  \n",__FUNCTION__,name);
		return NULL;
	}
	orbisGlTextures[orbisGlTextureCounter].textureId=textureId;
	orbisGlTextures[orbisGlTextureCounter].width=width;
	orbisGlTextures[orbisGlTextureCounter].height=height;
	strcpy(orbisGlTextures[orbisGlTextureCounter].name,name);
	debugNetPrintf(DEBUG,"[ORBISGL] %s Texture registered name=%s id=%d !\n",__FUNCTION__,name,textureId);
	orbisGlTextureCounter++;
	return &orbisGlTextures[orbisGlTextureCounter-1];
}

OrbisGlTexture * orbisGlTextureManagerRegisterPngTexture(char *name, char *path)
{
	debugNetPrintf(DEBUG,"[ORBISGL] %s \n",__FUNCTION__);
	OrbisRawTexture *texture_raw=orbisGlLoadPngFromNfs(path);
	if(!texture_raw)
	{
		debugNetPrintf(ERROR,"[ORBISGL] %s Texture can't be loaded %s, skip ... \n",__FUNCTION__,path);
		return NULL;
	}
	debugNetPrintf(DEBUG,"[ORBISGL] %s png loaded %s, ...\n",__FUNCTION__,path);
	GLenum format=GL_RGBA;
	if(texture_raw->depth==8)
	{
		format=GL_RGB;
	}
	debugNetPrintf(DEBUG,"[ORBISGL] %s rgba=%d rgb=%d\n",__FUNCTION__,format==GL_RGBA,format==GL_RGB);
	OrbisGlTexture *t=orbisGlTextureManagerRegisterTextureFromBuffer(name,texture_raw->width,texture_raw->height,format,texture_raw->datap);
	return t;
}
void orbisGlTextureManagerFinish()
{
	debugNetPrintf(DEBUG,"[ORBISGL] %s \n",__FUNCTION__);
	int i;
	for(i=0;i<orbisGlProgramCounter;i++)
	{
		orbisGlDestroyProgram(orbisGlPrograms[i].programId);
	}
	for(i=0;i<orbisGlTextureCounter;i++)
	{
		orbisGlDestroyProgram(orbisGlTextures[i].textureId);
	}
}
GLuint orbisGlCreateProgramFromNfs(const char* vertexShaderFilename, const char* fragmentShaderFilename)
{
	debugNetPrintf(DEBUG,"[ORBISGL] %s %s %s\n",__FUNCTION__,vertexShaderFilename,fragmentShaderFilename);
	char * vsSource=(char *)orbisNfsGetFileContent(vertexShaderFilename);
	if(vsSource==NULL)
	{
		debugNetPrintf(ERROR,"[ORBISGL] %s can't open vertex shader at %s\n",__FUNCTION__,vertexShaderFilename);
		return 0;
	}
	GLuint vs=orbisGlCompileShader(GL_VERTEX_SHADER,vsSource);
	if(vs==0)
	{
		debugNetPrintf(ERROR,"[ORBISGL] %s can't compile vertex shader at %s\n",__FUNCTION__,vertexShaderFilename);
		return 0;
	}
	free(vsSource);
	char * fsSource=(char *)orbisNfsGetFileContent(fragmentShaderFilename);
	if(fsSource==NULL)
	{
		debugNetPrintf(ERROR,"[ORBISGL] %s can't open fragment shader at %s\n",__FUNCTION__,vertexShaderFilename);
		return 0;
	}
	GLuint fs=orbisGlCompileShader(GL_FRAGMENT_SHADER,fsSource);
	if(fs==0)
	{
		debugNetPrintf(ERROR,"[ORBISGL] %s can't compile fragment shader at %s\n",__FUNCTION__,vertexShaderFilename);
	}
	free(fsSource);
	GLuint po=orbisGlLinkProgram(vs,fs);
	if(po==0)
	{
		debugNetPrintf(ERROR,"[ORBISGL] %s can't link program with vertex shader %d and fragment shader %d\n",__FUNCTION__,vs,fs);
		return 0;
	}
	glDeleteShader(vs);
	glDeleteShader(fs);
	
	return po;
}
OrbisGlProgram *orbisGlTextureManagerInit()
{
	debugNetPrintf(DEBUG,"[ORBISGL] %s \n",__FUNCTION__);
	OrbisGlProgram *p;
	GLuint programId=0;
	programId=orbisGlCreateProgramFromNfs(TEXTURE_VERTEX_SHADER,TEXTURE_FRAGMENT_SHADER);
	if(!programId)
	{
		debugNetPrintf(ERROR,"[ORBISGL] %s Error during linking shader ! program_id=%d (0x%08x)\n",__FUNCTION__,programId,programId);
		return 0;
	}
	debugNetPrintf(DEBUG,"[ORBISGL] %s linked shader program_id=%d (0x%08x)\n",__FUNCTION__,programId,programId);
	orbisGlPrograms[orbisGlProgramCounter].programId=programId;
	p=&orbisGlPrograms[orbisGlProgramCounter];
	orbisGlProgramCounter++;
	return p;
}

OrbisGlTextureState * orbisGlInitTexture(char *name,char *path,OrbisGlProgram *program,int x, int y)
{
	debugNetPrintf(DEBUG,"[ORBISGL] %s \n",__FUNCTION__);

	OrbisGlTextureState *p;
	p=(OrbisGlTextureState*)malloc(sizeof(OrbisGlTextureState));
	//  p->screenWidth=x;
	//  p->screenHeight=y;
	p->texture=orbisGlTextureManagerRegisterPngTexture(name,path);
	debugNetPrintf(DEBUG,"[ORBISGL] %s after texture register program %d\n",__FUNCTION__,program?1:0);

	if(p->texture==NULL || program==NULL)
	{
		if(p->texture==NULL)
		{
			debugNetPrintf(ERROR,"[ORBISGL] %s texture is NULL 0\n",__FUNCTION__);
		}
		else
		{
			debugNetPrintf(ERROR,"[ORBISGL] %s texture is NULL 1\n",__FUNCTION__);
		}
		//free(p);
		return NULL;
	}
	//debugNetPrintf(DEBUG,"[MSXORBIS] %s after test program=%d %d %d\n",__FUNCTION__,program?1:0,p->screenWidth,p->screenHeight);

	p->program=program;
	debugNetPrintf(DEBUG,"[ORBISGL] %s after test program=%d\n",__FUNCTION__,p->program?1:0);

	GLuint programId=orbisGlGetProgramId(p->program);
	debugNetPrintf(DEBUG,"[ORBISGL] %s programId %d\n",__FUNCTION__, programId);

	p->s_xyz_loc     = glGetAttribLocation(programId,  "a_xyz");
	p->s_uv_loc      = glGetAttribLocation(programId,  "a_uv");
	p->s_sampler_loc = glGetUniformLocation(programId, "s_texture");

	// set invariant uniforms
	// glUseProgram(programId);
	// glUniform1i(p->samplerId, 0); // sample from texture unit #0
	// glUniform2f(p->screenHalfSizeId, (float)p->screenWidth/2, (float)p->screenHeight/2);
	// glUseProgram(0);

	// allocate a buffer for the  vertex data
	// p->bufferData = (OrbisGlVertexInfo*)malloc(sizeof(OrbisGlVertexInfo)*6);
	p->count = 0;
	//  debugNetPrintf(DEBUG,"[MSXORBIS] %s samplerId %d\n",__FUNCTION__, p->samplerId);

	return p;
}

void orbisGlFinishTexture(OrbisGlTextureState *p)
{
	debugNetPrintf(DEBUG,"[ORBISGL] %s \n",__FUNCTION__);
	glDeleteBuffers(1, &p->bufferId);
	glDeleteTextures(1, &p->texture->textureId);
	free(p->bufferData);
}
void orbisGlTextureAdd(OrbisGlTextureState *p, int x, int y,int w,int h)
{
	debugNetPrintf(DEBUG,"[ORBISGL] %s \n",__FUNCTION__);

	float fx=(float)x;
	float fy=(float)y;
	float fw=(float)w;
	float fh=(float)h;

	// fill vertex/uv buffer 
	OrbisGlVertexInfo* pBuffer = &p->bufferData[p->count*6];  
    
	*pBuffer++ = (OrbisGlVertexInfo) { fx, fy, 0.0, 0.0 };
	*pBuffer++ = (OrbisGlVertexInfo) { fx, fy+fh, 0.0, 1.0 };
	*pBuffer++ = (OrbisGlVertexInfo) { fx+fw, fy, 1.0, 0.0 };
	*pBuffer++ = (OrbisGlVertexInfo) { fx+fw, fy+fh, 1.0, 1.0 };
	*pBuffer++ = (OrbisGlVertexInfo) { fx+fw, fy, 1.0, 0.0 };
	*pBuffer++ = (OrbisGlVertexInfo) { fx, fy+fh, 0.0, 1.0 };
	p->count++;
}
void orbisGlDrawTextureSpecial2(OrbisGlTextureState *p, int x, int y)
{
	//debugNetPrintf(DEBUG,"[ORBISGL] %s \n",__FUNCTION__);

	//  float fx = x;
	//  float fy = y;
	if (!p)
		return;
	float w=1920.0;
	float h=1080.0;
	int x1=x+p->texture->width;
	int y1=y+p->texture->height;

	/*float fx=2.0*(x/w)-1.0;
	float fy=2.0*((1080-y)/h)-1.0;
	float fx1=2.0*(x1/w)-1.0;
	float fy1=2.0*((1080-y1)/h)-1.0;
	const GLfloat vertexArray[] = 
	{
		fx,  fy, 0.0f, // Position 0
		fx, fy+0,1, 0.0f, // Position 1
		fx+0.1,fy+0.1, 0.0f, // Position 2
		fx+0.1,  fy, 0.0f, // Position 3
	};*/
	const GLfloat vertexArray[] = 
	{
		-0.01f,  0.01f, 0.0f, // Position 0
		-0.01f, -0.01f, 0.0f, // Position 1
		0.01f, -0.01f, 0.0f, // Position 2
		0.01f,  0.01f, 0.0f, // Position 3
	};

	const GLfloat textureArray[] = 
	{
		0.0f,  0.0f, // Position 0
		0.0f,  1.0f, // Position 1
		1.0f,  1.0f, // Position 2
		1.0f,  0.0f, // Position 3
	};

	const uint16_t drawList[] = { 0, 1, 2, 0, 2, 3 };

	int ret = 0;
	GLuint programId=orbisGlGetProgramId(p->program);
	GLuint textureId=orbisGlGetTextureId(p->texture);
    
	glUseProgram(programId);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glVertexAttribPointer(p->s_xyz_loc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)vertexArray);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBISGL] glVertexAttribPointer failed: 0x%08X\n", ret);
	}

	glVertexAttribPointer(p->s_uv_loc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)textureArray);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBISGL] glVertexAttribPointer failed: 0x%08X\n", ret);
	}

	glEnableVertexAttribArray(p->s_xyz_loc);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBISGL] glEnableVertexAttribArray (1) failed: 0x%08X\n", ret);
	}

	glEnableVertexAttribArray(p->s_uv_loc);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBISGL] glEnableVertexAttribArray (2) failed: 0x%08X\n", ret);
	}

	glActiveTexture(GL_TEXTURE0);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBISGL] glActiveTexture failed: 0x%08X\n", ret);
	}

	glBindTexture(GL_TEXTURE_2D, textureId);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBISGL] glBindTexture failed: 0x%08X\n", ret);
	}

	glUniform1i(p->s_sampler_loc, 0);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBISGL] glUniform1i failed: 0x%08X\n", ret);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, drawList);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBISGL] glDrawElements failed: 0x%08X\n", ret);
	}
	// disconnect slots from shader
	glDisableVertexAttribArray(p->s_xyz_loc);
	glDisableVertexAttribArray(p->s_uv_loc);
	glUseProgram(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND);
}

void orbisGlDrawTextureSpecial(OrbisGlTextureState *p, int x, int y)
{
	//debugNetPrintf(DEBUG,"[ORBISGL] %s \n",__FUNCTION__);

	//  float fx = x;
	//  float fy = y;
	if (!p)
		return;
	float w=1920.0;
	float h=1080.0;
	int x1=x+p->texture->width;
	int y1=y+p->texture->height;

	float fx=2.0*(x/w)-1.0;
	float fy=2.0*((1080-y)/h)-1.0;
	float fx1=2.0*(x1/w)-1.0;
	float fy1=2.0*((1080-y1)/h)-1.0;
	const GLfloat vertexArray[] = 
	{
		fx,  fy, 0.0f, // Position 0
		fx, fy1, 0.0f, // Position 1
		fx1,fy1, 0.0f, // Position 2
		fx1, fy, 0.0f, // Position 3
	};
	/*const GLfloat vertexArray[] = 
	{
		-0.01f,  0.01f, 0.0f, // Position 0
		-0.01f, -0.01f, 0.0f, // Position 1
		0.01f, -0.01f, 0.0f, // Position 2
		0.01f,  0.01f, 0.0f, // Position 3
	};*/

	const GLfloat textureArray[] = 
	{
		0.0f,  0.0f, // Position 0
		0.0f,  1.0f, // Position 1
		1.0f,  1.0f, // Position 2
		1.0f,  0.0f, // Position 3
	};

	const uint16_t drawList[] = { 0, 1, 2, 0, 2, 3 };

	int ret = 0;
	GLuint programId=orbisGlGetProgramId(p->program);
	GLuint textureId=orbisGlGetTextureId(p->texture);
    
	glUseProgram(programId);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glVertexAttribPointer(p->s_xyz_loc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)vertexArray);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBISGL] glVertexAttribPointer failed: 0x%08X\n", ret);
	}

	glVertexAttribPointer(p->s_uv_loc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)textureArray);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBISGL] glVertexAttribPointer failed: 0x%08X\n", ret);
	}

	glEnableVertexAttribArray(p->s_xyz_loc);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBISGL] glEnableVertexAttribArray (1) failed: 0x%08X\n", ret);
	}

	glEnableVertexAttribArray(p->s_uv_loc);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBISGL] glEnableVertexAttribArray (2) failed: 0x%08X\n", ret);
	}

	glActiveTexture(GL_TEXTURE0);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBISGL] glActiveTexture failed: 0x%08X\n", ret);
	}

	glBindTexture(GL_TEXTURE_2D, textureId);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBISGL] glBindTexture failed: 0x%08X\n", ret);
	}

	glUniform1i(p->s_sampler_loc, 0);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBISGL] glUniform1i failed: 0x%08X\n", ret);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, drawList);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBISGL] glDrawElements failed: 0x%08X\n", ret);
	}
	// disconnect slots from shader
	glDisableVertexAttribArray(p->s_xyz_loc);
	glDisableVertexAttribArray(p->s_uv_loc);
	glUseProgram(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND);
}


void orbisGlDrawTexture(OrbisGlTextureState *p) 
{	
	debugNetPrintf(0,"[ORBISGL] %s \n",__FUNCTION__);

	//  float fx = x;	
	//  float fy = y;

	if (!p)
		return;

	const GLfloat vertexArray[] = 
	{
		-1.0f,  1.0f, 0.0f, // Position 0
		-1.0f, -1.0f, 0.0f, // Position 1
		1.0f, -1.0f, 0.0f, // Position 2
		1.0f,  1.0f, 0.0f, // Position 3
	};

	const GLfloat textureArray[] = 
	{
		0.0f,  0.0f, // Position 0
		0.0f,  1.0f, // Position 1
		1.0f,  1.0f, // Position 2
		1.0f,  0.0f, // Position 3
	};

	const uint16_t drawList[] = { 0, 1, 2, 0, 2, 3 };

	int ret = 0;
	GLuint programId=orbisGlGetProgramId(p->program);
	GLuint textureId=orbisGlGetTextureId(p->texture);
    
	glUseProgram(programId);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glVertexAttribPointer(p->s_xyz_loc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)vertexArray);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBISGL] glVertexAttribPointer failed: 0x%08X\n", ret);
	}

	glVertexAttribPointer(p->s_uv_loc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)textureArray);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBISGL] glVertexAttribPointer failed: 0x%08X\n", ret);
	}

	glEnableVertexAttribArray(p->s_xyz_loc);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBISGL] glEnableVertexAttribArray (1) failed: 0x%08X\n", ret);
	}

	glEnableVertexAttribArray(p->s_uv_loc);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBISGL] glEnableVertexAttribArray (2) failed: 0x%08X\n", ret);
	}

	glActiveTexture(GL_TEXTURE0);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBISGL] glActiveTexture failed: 0x%08X\n", ret);
	}

	glBindTexture(GL_TEXTURE_2D, textureId);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBISGL] glBindTexture failed: 0x%08X\n", ret);
	}

	glUniform1i(p->s_sampler_loc, 0);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBISGL] glUniform1i failed: 0x%08X\n", ret);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, drawList);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBISGL] glDrawElements failed: 0x%08X\n", ret);
	}
	// disconnect slots from shader
	glDisableVertexAttribArray(p->s_xyz_loc);
	glDisableVertexAttribArray(p->s_uv_loc);
	glUseProgram(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND);
}

















