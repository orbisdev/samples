/*
 *	debugnet library sample for PlayStation 4 
 *	Copyright (C) 2010,2016 Antonio Jose Ramos Marquez (aka bigboss) @psxdev on twitter
 *  Repository https://github.com/psxdev/ps4link
 */


#include <stdio.h>
#include <stdlib.h>

#include <kernel.h>
#include <systemservice.h>
#include <orbis2d.h>
#include <orbisPad.h>
#include <orbisKeyboard.h>
#include <orbisAudio.h>
#include <modplayer.h>
#include <ps4link.h>
#include <debugnet.h>
#include <piglet.h>

Orbis2dConfig *conf;
OrbisPadConfig *confPad;

typedef struct OrbisGlobalConf
{
	Orbis2dConfig *conf;
	OrbisPadConfig *confPad;
	OrbisAudioConfig *confAudio;
	OrbisKeyboardConfig *confKeyboard;
	ps4LinkConfiguration *confLink;
	int orbisLinkFlag;
}OrbisGlobalConf;

OrbisGlobalConf *myConf;

#define RENDER_WIDTH 1920
#define RENDER_HEIGHT 1080

static EGLDisplay s_display = EGL_NO_DISPLAY;
static EGLSurface s_surface = EGL_NO_SURFACE;
static EGLContext s_context = EGL_NO_CONTEXT;

static GLuint s_texture_id = 0;
static GLuint s_program_id = 0;

static GLint s_xyz_loc;
static GLint s_uv_loc;
static GLint s_sampler_loc;

static const GLfloat s_obj_vertices[] = {
	-0.5f, 0.5f, 0.0f,  /* XYZ #0 */
	 0.0f, 0.0f,        /* UV  #0 */
	-0.5f, -0.5f, 0.0f, /* XYZ #1 */
	 0.0f,  1.0f,       /* UV  #1 */
	 0.5f, -0.5f, 0.0f, /* XYZ #2 */
	 1.0f,  1.0f,       /* UV  #2 */
	 0.5f,  0.5f, 0.0f, /* XYZ #3 */
	 1.0f,  0.0f        /* UV  #3 */
};
static const GLushort s_obj_indices[] = {
	0, 1, 2, 0, 2, 3,
};

static const GLubyte s_texture_data[4 * 3] = {
	255, 0, 0,   /* red */
	0, 255, 0,   /* green */
	0, 0, 255,   /* blue */
	255, 255, 0, /* yellow */
};

static const GLchar s_vertex_shader_code[] =
	"attribute vec4 a_xyz;\n"
	"attribute vec2 a_uv;\n"
	"varying vec2 v_uv;\n"
	"\n"
	"void main() {\n"
	"	gl_Position = a_xyz;\n"
	"	v_uv = a_uv;\n"
	"}\n";

static const GLchar s_fragment_shader_code[] =
	"precision mediump float;\n"
	"varying vec2 v_uv;\n"
	"uniform sampler2D s_texture;\n"
	"\n"
	"void main() {\n"
	"	gl_FragColor = texture2D(s_texture, v_uv);\n"
"}\n";

static bool create_context(unsigned int width, unsigned int height) 
{
	ScePglConfig pgl_config;
	SceWindow render_window = { 0, width, height };
	EGLConfig config = NULL;
	EGLint num_configs;

	EGLint attribs[] = {
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_DEPTH_SIZE, 0,
		EGL_STENCIL_SIZE, 0,
		EGL_SAMPLE_BUFFERS, 0,
		EGL_SAMPLES, 0,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_NONE,
	};

	EGLint ctx_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE,
	};

	EGLint window_attribs[] = {
		EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
		EGL_NONE,
	};

	int major, minor;
	int ret;

	memset(&pgl_config, 0, sizeof(pgl_config));
	{
		pgl_config.size = sizeof(pgl_config);
		pgl_config.flags = SCE_PGL_FLAGS_USE_COMPOSITE_EXT | SCE_PGL_FLAGS_USE_FLEXIBLE_MEMORY | 0x60;
		pgl_config.processOrder = 1;
		pgl_config.systemSharedMemorySize = 0x200000;
		pgl_config.videoSharedMemorySize = 0x2400000;
		pgl_config.maxMappedFlexibleMemory = 0xAA00000;
		pgl_config.drawCommandBufferSize = 0xC0000;
		pgl_config.lcueResourceBufferSize = 0x10000;
		pgl_config.dbgPosCmd_0x40 = 1920;
		pgl_config.dbgPosCmd_0x44 = 1080;
		pgl_config.dbgPosCmd_0x48 = 0;
		pgl_config.dbgPosCmd_0x4C = 0;
		pgl_config.unk_0x5C = 2;
	}

	if (!scePigletSetConfigurationVSH(&pgl_config)) {
		debugNetPrintf(ERROR,"[ORBIS_GL] scePigletSetConfigurationVSH failed.\n");
		goto err;
	}

	s_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (s_display == EGL_NO_DISPLAY) {
		debugNetPrintf(ERROR,"[ORBIS_GL] eglGetDisplay failed.\n");
		goto err;
	}

	if (!eglInitialize(s_display, &major, &minor)) {
		ret = eglGetError();
		debugNetPrintf(ERROR,"[ORBIS_GL] eglInitialize failed: 0x%08X\n", ret);
		goto err;
	}
	printf("EGL version major:%d, minor:%d\n", major, minor);

	if (!eglBindAPI(EGL_OPENGL_ES_API)) {
		ret = eglGetError();
		debugNetPrintf(ERROR,"[ORBIS_GL] eglBindAPI failed: 0x%08X\n", ret);
		goto err;
	}

	if (!eglSwapInterval(s_display, 0)) {
		ret = eglGetError();
		debugNetPrintf(ERROR,"[ORBIS_GL] eglSwapInterval failed: 0x%08X\n", ret);
		goto err;
	}

	if (!eglChooseConfig(s_display, attribs, &config, 1, &num_configs)) {
		ret = eglGetError();
		debugNetPrintf(ERROR,"[ORBIS_GL] eglChooseConfig failed: 0x%08X\n", ret);
		goto err;
	}
	if (num_configs != 1) {
		debugNetPrintf(ERROR,"[ORBIS_GL] No available configuration found.\n");
		goto err;
	}

	s_surface = eglCreateWindowSurface(s_display, config, &render_window, window_attribs);
	if (s_surface == EGL_NO_SURFACE) {
		ret = eglGetError();
		debugNetPrintf(ERROR,"[ORBIS_GL] eglCreateWindowSurface failed: 0x%08X\n", ret);
		goto err;
	}

	s_context = eglCreateContext(s_display, config, EGL_NO_CONTEXT, ctx_attribs);
	if (s_context == EGL_NO_CONTEXT) {
		ret = eglGetError();
		debugNetPrintf(ERROR,"[ORBIS_GL] eglCreateContext failed: 0x%08X\n", ret);
		goto err;
	}

	if (!eglMakeCurrent(s_display, s_surface, s_surface, s_context)) {
		ret = eglGetError();
		debugNetPrintf(ERROR,"[ORBIS_GL] eglMakeCurrent failed: 0x%08X\n", ret);
		goto err;
	}

	debugNetPrintf(INFO,"[ORBIS_GL] GL_VERSION: %s\n", glGetString(GL_VERSION));
	debugNetPrintf(INFO,"[ORBIS_GL] GL_RENDERER: %s\n", glGetString(GL_RENDERER));

	return true;

err:
	return false;
}

static void destroy_context(void) {
	int ret;

	if (!eglDestroyContext(s_display, s_context)) {
		ret = eglGetError();
		debugNetPrintf(ERROR,"[ORBIS_GL] eglDestroyContext failed: 0x%08X\n", ret);
	}
	s_context = EGL_NO_CONTEXT;

	if (!eglDestroySurface(s_display, s_surface)) {
		ret = eglGetError();
		debugNetPrintf(ERROR,"[ORBIS_GL] eglDestroySurface failed: 0x%08X\n", ret);
	}
	s_surface = EGL_NO_SURFACE;

	if (!eglTerminate(s_display)) {
		ret = eglGetError();
		debugNetPrintf(ERROR,"[ORBIS_GL] eglTerminate failed: 0x%08X\n", ret);
	}
	s_display = EGL_NO_DISPLAY;
}

static bool create_texture(void) {
	int ret;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBIS_GL] glPixelStorei failed: 0x%08X\n", ret);
		goto err;
	}

	glGenTextures(1, &s_texture_id);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBIS_GL] glGenTextures failed: 0x%08X\n", ret);
		goto err;
	}

	glBindTexture(GL_TEXTURE_2D, s_texture_id);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBIS_GL] glBindTexture failed: 0x%08X\n", ret);
		goto err;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, s_texture_data);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBIS_GL] glTexImage2D failed: 0x%08X\n", ret);
		goto err;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBIS_GL] glTexParameteri failed: 0x%08X\n", ret);
		goto err;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBIS_GL] glTexParameteri failed: 0x%08X\n", ret);
		goto err;
	}

	return true;

err:
	return false;
}

static bool destroy_texture(void) {
	int ret;

	glDeleteTextures(1, &s_texture_id);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBIS_GL] glDeleteTextures failed: 0x%08X\n", ret);
		goto err;
	}

	return true;

err:
	return false;
}

static bool compile_shader(GLenum type, const char* source, size_t length, GLuint* pShader) {
	GLuint shader;
	GLint tmp_length = (int)length;
	GLint success;
	char log_buf[256];
	int ret;

	shader = glCreateShader(type);
	if (!shader) {
		ret = glGetError();
		debugNetPrintf(ERROR,"[ORBIS_GL] glCreateShader failed: 0x%08X\n", ret);
		goto err;
	}

	glShaderSource(shader, 1, &source, &tmp_length);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBIS_GL] glShaderSource failed: 0x%08X\n", ret);
		goto err;
	}

	glCompileShader(shader);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBIS_GL] glCompileShader failed: 0x%08X\n", ret);
		goto err;
	}

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBIS_GL] glGetShaderiv failed: 0x%08X\n", ret);
		goto err;
	}
	if (!success) {
		glGetShaderInfoLog(shader, sizeof(log_buf), NULL, log_buf);
		ret = glGetError();
		if (ret) {
			debugNetPrintf(ERROR,"[ORBIS_GL] glGetShaderInfoLog failed: 0x%08X\n", ret);
			goto err;
		}
		if (strlen(log_buf) > 1) {
			debugNetPrintf(ERROR,"[ORBIS_GL] shader compilation failed with log:\n%s\n", log_buf);
		} else {
			debugNetPrintf(ERROR,"[ORBIS_GL] shader compilation failed\n");
		}
		goto err;
	}

	if (pShader) {
		*pShader = shader;
	}

	return true;

err:
	return false;
}

static bool create_program(void) {
	GLuint vertex_shader_id = 0;
	GLuint fragment_shader_id = 0;
	GLuint program_id = 0;
	GLint success;
	char log[256];
	int ret;

	if (!compile_shader(GL_VERTEX_SHADER, s_vertex_shader_code, strlen(s_vertex_shader_code), &vertex_shader_id)) {
		debugNetPrintf(ERROR,"[ORBIS_GL] Unable to compile vertex shader.\n");
		goto err;
	}

	if (!compile_shader(GL_FRAGMENT_SHADER, s_fragment_shader_code, strlen(s_fragment_shader_code), &fragment_shader_id)) {
		debugNetPrintf(ERROR,"[ORBIS_GL] Unable to compile fragment shader.\n");
		goto err;
	}

	program_id = glCreateProgram();
	if (!program_id) {
		ret = glGetError();
		debugNetPrintf(ERROR,"[ORBIS_GL] glCreateProgram failed: 0x%08X\n", ret);
		goto err;
	}

	glAttachShader(program_id, vertex_shader_id);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBIS_GL] glAttachShader(vertex_shader) failed: 0x%08X\n", ret);
		goto err;
	}

	glAttachShader(program_id, fragment_shader_id);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBIS_GL] glAttachShader(fragment_shader) failed: 0x%08X\n", ret);
		goto err;
	}

	glLinkProgram(program_id);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBIS_GL] glLinkProgram() failed: 0x%08X\n", ret);
		goto err;
	}

	glGetProgramiv(program_id, GL_LINK_STATUS, &success);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBIS_GL] glGetProgramiv() failed: 0x%08X\n", ret);
		goto err;
	}
	if (!success) {
		glGetProgramInfoLog(program_id, sizeof(log), NULL, log);
		ret = glGetError();
		if (ret) {
			debugNetPrintf(ERROR,"[ORBIS_GL] glGetProgramInfoLog failed: 0x%08X\n", ret);
			goto err;
		}
		if (strlen(log) > 1) {
			debugNetPrintf(ERROR,"[ORBIS_GL] Unable to link shader program:\n%s\n", log);
		} else {
			debugNetPrintf(ERROR,"[ORBIS_GL] Unable to link shader program.\n");
		}
		goto err;
	}

	glDeleteShader(fragment_shader_id);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBIS_GL] glDeleteShader(fragment_shader) failed: 0x%08X\n", ret);
		goto err;
	}

	glDeleteShader(vertex_shader_id);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBIS_GL] glDeleteShader(vertex_shader) failed: 0x%08X\n", ret);
		goto err;
	}

	s_xyz_loc = glGetAttribLocation(program_id, "a_xyz");
	s_uv_loc = glGetAttribLocation(program_id, "a_uv");
	s_sampler_loc = glGetUniformLocation(program_id, "s_texture");

	s_program_id = program_id;

	return true;

err:
	if (program_id > 0) {
		glDeleteProgram(program_id);
		ret = glGetError();
		if (ret) {
			debugNetPrintf(ERROR,"[ORBIS_GL] glDeleteProgram failed: 0x%08X\n", ret);
			goto err;
		}
	}

	if (fragment_shader_id > 0) {
		glDeleteShader(fragment_shader_id);
		ret = glGetError();
		if (ret) {
			debugNetPrintf(ERROR,"[ORBIS_GL] glDeleteShader(fragment_shader) failed: 0x%08X\n", ret);
			goto err;
		}
		fragment_shader_id = 0;
	}

	if (vertex_shader_id > 0) {
		glDeleteShader(vertex_shader_id);
		ret = glGetError();
		if (ret) {
			debugNetPrintf(ERROR,"[ORBIS_GL] glDeleteShader(vertex_shader) failed: 0x%08X\n", ret);
			goto err;
		}
		vertex_shader_id = 0;
	}

	return false;
}

static bool destroy_program(void) {
	int ret;

	glDeleteProgram(s_program_id);
	ret = glGetError();
	if (ret) {
		debugNetPrintf(ERROR,"[ORBIS_GL] glDeleteProgram failed: 0x%08X\n", ret);
		goto err;
	}

	return true;

err:
	return false;
}

static bool main_loop(void) {
	int ret;

	while (1) {
		glClear(GL_COLOR_BUFFER_BIT);
		ret = glGetError();
		if (ret) {
			debugNetPrintf(ERROR,"[ORBIS_GL] glClear failed: 0x%08X\n", ret);
			goto err;
		}

		glUseProgram(s_program_id);
		ret = glGetError();
		if (ret) {
			debugNetPrintf(ERROR,"[ORBIS_GL] glUseProgram failed: 0x%08X\n", ret);
			goto err;
		}

		glVertexAttribPointer(s_xyz_loc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), s_obj_vertices);
		ret = glGetError();
		if (ret) {
			debugNetPrintf(ERROR,"[ORBIS_GL] glVertexAttribPointer failed: 0x%08X\n", ret);
			goto err;
		}
		glVertexAttribPointer(s_uv_loc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &s_obj_vertices[3]);
		ret = glGetError();
		if (ret) {
			debugNetPrintf(ERROR,"[ORBIS_GL] glVertexAttribPointer failed: 0x%08X\n", ret);
			goto err;
		}

		glEnableVertexAttribArray(s_xyz_loc);
		ret = glGetError();
		if (ret) {
			debugNetPrintf(ERROR,"[ORBIS_GL] glEnableVertexAttribArray failed: 0x%08X\n", ret);
			goto err;
		}
		glEnableVertexAttribArray(s_uv_loc);
		ret = glGetError();
		if (ret) {
			debugNetPrintf(ERROR,"[ORBIS_GL] glEnableVertexAttribArray failed: 0x%08X\n", ret);
			goto err;
		}

		glActiveTexture(GL_TEXTURE0);
		ret = glGetError();
		if (ret) {
			debugNetPrintf(ERROR,"[ORBIS_GL] glActiveTexture failed: 0x%08X\n", ret);
			goto err;
		}
		glBindTexture(GL_TEXTURE_2D, s_texture_id);
		ret = glGetError();
		if (ret) {
			debugNetPrintf(ERROR,"[ORBIS_GL] glBindTexture failed: 0x%08X\n", ret);
			goto err;
		}

		glUniform1i(s_sampler_loc, 0);
		ret = glGetError();
		if (ret) {
			debugNetPrintf(ERROR,"[ORBIS_GL] glUniform1i failed: 0x%08X\n", ret);
			goto err;
		}

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, s_obj_indices);
		ret = glGetError();
		if (ret) {
			debugNetPrintf(ERROR,"[ORBIS_GL] glDrawElements failed: 0x%08X\n", ret);
			goto err;
		}

		if (!eglSwapBuffers(s_display, s_surface)) {
			ret = eglGetError();
			debugNetPrintf(ERROR,"[ORBIS_GL] eglSwapBuffers failed: 0x%08X\n", ret);
			goto err;
		}
	}

	return true;

err:
	return false;
}




int main(int argc, char *argv[])
{
	int ret;


	uintptr_t intptr=0;
	sscanf(argv[1],"%p",&intptr);
	myConf=(OrbisGlobalConf *)intptr;
	ret=ps4LinkInitWithConf(myConf->confLink);
	if(!ret)
	{
		ps4LinkFinish();
		return 0;
	}
	debugNetPrintf(INFO,"[ORBIS_GL] Hello from first gl es sample with hitodama's sdk and liborbis\n");	
	sleep(1);

	ret=sceSystemServiceHideSplashScreen();
	if(ret) 
	{
		debugNetPrintf(ERROR,"[ORBIS_GL] sceSystemServiceHideSplashScreen failed: 0x%08X\n", ret);
		goto err;
	}

	if(!create_context(RENDER_WIDTH, RENDER_HEIGHT)) 
	{
		debugNetPrintf(ERROR,"[ORBIS_GL] Unable to create context.\n");
		goto err;
	}
	if(!create_texture()) 
	{
		debugNetPrintf(ERROR,"[ORBIS_GL] Unable to create texture.\n");
		goto err_destroy_context;
	}
	if(!create_program())
	{
		debugNetPrintf(ERROR,"[ORBIS_GL] Unable to create shader program.\n");
		goto err_destroy_texture;
	}

	glViewport(0, 0, RENDER_WIDTH, RENDER_HEIGHT);
	ret = glGetError();
	if(ret)
	{
		debugNetPrintf(ERROR,"[ORBIS_GL] glViewport failed: 0x%08X\n", ret);
		goto err_destroy_program;
	}

	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
	ret = glGetError();
	if (ret) 
	{
		debugNetPrintf(ERROR,"[ORBIS_GL] glClearColor failed: 0x%08X\n", ret);
		goto err_destroy_program;
	}

	if (!main_loop()) 
	{
		debugNetPrintf(ERROR,"[ORBIS_GL] Main loop failed.\n");
		goto err_destroy_program;
	}

	err_destroy_program:
		destroy_program();

	err_destroy_texture:
		destroy_texture();

	err_destroy_context:
		destroy_context();

	err:
	
	
	ps4LinkFinish();
	
	return EXIT_SUCCESS;
}
