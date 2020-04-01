/*
PL_MPEG Example - Video player using SDL2/OpenGL for rendering

Dominic Szablewski - https://phoboslab.org


-- LICENSE: The MIT License(MIT)

Copyright(c) 2019 Dominic Szablewski

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files(the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions :
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


-- Usage

plmpeg-player <video-file.mpg>


-- About

This program demonstrates a simple video/audio player using plmpeg for decoding
and SDL2 with OpenGL for rendering and sound output. It was tested on Windows
using Microsoft Visual Studio 2015 and on macOS using XCode 10.2

This program can be configured to either convert the raw YCrCb data to RGB on
the GPU (default), or to do it on CPU. Just pass APP_TEXTURE_MODE_RGB to
app_create() to switch to do the conversion on the CPU.

YCrCb->RGB conversion on the CPU is a very costly operation and should be
avoided if possible. It easily takes as much time as all other mpeg1 decoding
steps combined.

*/

/*
 clang pl_mpeg_player.c -D__LINUX__ -lm -lGL -lGLEW -lSDL2
 clang pl_mpeg_player.c -D__LINUX__ -lm -lEGL -lGL -lSDL2
 clang pl_mpeg_player.c -D__LINUX__ -lm -lEGL -lGL -lGLESv2 -lSDL2
 clang pl_mpeg_player.c -D__LINUX__ -lm -lEGL -lGLESv2 -lSDL2 -lGL

clang pl_mpeg_player.c -D__LINUX__ -lm -lEGL -lGLESv2 -lSDL2 -O3
clang pl_mpeg_player.c -D__LINUX__ -lm -lGLESv2 -lSDL2 -O3
clang pl_mpeg_player.c -D__LINUX__ -lm -lGLESv2 -lSDL2 -O3 -ggdb

 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

/*	egrep 'MIN\(|MAX\(' /usr/include/sys/param.h */
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

	#define GL3_PROTOTYPES 1

	#include  <GLES2/gl2.h>
	#include  <GLES2/gl2ext.h>
	#include  <EGL/egl.h>

#if defined _LIBAO_
	#include <ao/ao.h>
	ao_device *device;
	ao_sample_format format;
	int default_driver;

#endif

void glCreateTextures(GLuint ignored, GLsizei n, GLuint *name) {
	glGenTextures(1, name);
}


#include "defines.h" // user_nfs.c, the libnfs part


/// mpeg2 video decoding
#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg.h"


/// mpeg2 audio decoding, (we need just to turn_f32_to_s16le(samples)!)

#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"
#define DRMP3_HAVE_SSE 1 
#define DR_MP3_NO_STDIO


#define APP_SHADER_SOURCE(...) #__VA_ARGS__;

const char * const APP_VERTEX_SHADER = APP_SHADER_SOURCE(
	precision mediump float;

	attribute vec2 vertex;
	varying   vec2 tex_coord;
	
	void main() {
		tex_coord   = vertex;
		gl_Position = vec4((vertex * 2.0 - 1.0) * vec2(1, -1), 0.0, 1.0);
	}
);

const char * const APP_FRAGMENT_SHADER_YCRCB = APP_SHADER_SOURCE(
	precision mediump float;

	uniform sampler2D texture_y;
	uniform sampler2D texture_cb;
	uniform sampler2D texture_cr;

	varying vec2 tex_coord;

	mat4 rec601 = mat4(
		1.16438,  0.00000,  1.59603, -0.87079,
		1.16438, -0.39176, -0.81297,  0.52959,
		1.16438,  2.01723,  0.00000, -1.08139,
		0, 0, 0, 1
	);
	  
	void main() {
		float y  = texture2D(texture_y,  tex_coord).r;
		float cb = texture2D(texture_cb, tex_coord).r;
		float cr = texture2D(texture_cr, tex_coord).r;

		gl_FragColor = vec4(y, cb, cr, 1.0) * rec601;
	}
);

const char * const APP_FRAGMENT_SHADER_RGB = APP_SHADER_SOURCE(
	uniform sampler2D texture_rgb;

	varying vec2 tex_coord;

	void main() {
		gl_FragColor = vec4(texture2D(texture_rgb, tex_coord).rgb, 1.0);
	}
);

#undef APP_SHADER_SOURCE

#define APP_TEXTURE_MODE_YCRCB 1
#define APP_TEXTURE_MODE_RGB   2

typedef struct {
	plm_t *plm;
	double last_time;
	bool wants_to_quit;

#ifdef _SDL_
	SDL_Window *window;
	SDL_GLContext gl;
	SDL_AudioDeviceID audio_device;
#endif

	GLuint shader_program;
	GLuint vertex_shader;
	GLuint fragment_shader;
	
	int texture_mode;
	GLuint texture_y;
	GLuint texture_cb;
	GLuint texture_cr;
	
	GLuint texture_rgb;
	uint8_t *rgb_data;
} app_t;

app_t *app_create (const char *filename, int texture_mode);
void   app_update (app_t *self);
void   app_destroy(app_t *self);

GLuint app_compile_shader(app_t *self, GLenum type,  const char *source);
GLuint app_create_texture(app_t *self, GLuint index, const char *name);
void   app_update_texture(app_t *self, GLuint unit,  GLuint texture, plm_plane_t *plane);

void   app_on_video(plm_t *player, plm_frame_t   *frame,   void *user);
void   app_on_audio(plm_t *player, plm_samples_t *samples, void *user);


app_t *app_create(const char *filename, int texture_mode)
{
	app_t *self = (app_t *)malloc(sizeof(app_t));
	memset(self, 0, sizeof(app_t));
	
	self->texture_mode = texture_mode;
	
	// Initialize plmpeg, load the video file, install decode callbacks
	self->plm = plm_create_with_filename(filename);
	if (!self->plm) { printf("Couldn't open %s\n", filename); exit(1); }

	int samplerate = plm_get_samplerate(self->plm);

	printf("Opened %s - framerate: %f, samplerate: %d\n", filename, 
		plm_get_framerate (self->plm),
		plm_get_samplerate(self->plm)
	);
	
	plm_set_video_decode_callback(self->plm, app_on_video, self);
	plm_set_audio_decode_callback(self->plm, app_on_audio, self);
	
	plm_set_loop(self->plm, FALSE);
	plm_set_audio_enabled(self->plm, TRUE, 0);

	if (plm_get_num_audio_streams(self->plm) > 0)
	{
#ifdef _SDL_
		// Create SDL Window
		self->window = SDL_CreateWindow(
			"pl_mpeg",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			plm_get_width(self->plm), plm_get_height(self->plm),
			SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
		);
		self->gl = SDL_GL_CreateContext(self->window);
	
		SDL_GL_SetSwapInterval(1);

		// Initialize SDL Audio
		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
		SDL_AudioSpec audio_spec;
		SDL_memset(&audio_spec, 0, sizeof(audio_spec));
		audio_spec.freq = samplerate;
		audio_spec.format = AUDIO_F32;
		audio_spec.channels = 2;
		audio_spec.samples = 4096; // 

		self->audio_device = SDL_OpenAudioDevice(NULL, 0, &audio_spec, NULL, 0);

		printf("open audio device: %u\n", self->audio_device);
		if (self->audio_device == 0) { printf("Failed to open audio device: %s", SDL_GetError()); }
		SDL_PauseAudioDevice(self->audio_device, 0);

#elif defined _LIBAO_
		// -- Setup for default audio driver --
	    ao_initialize();
	    default_driver     = ao_default_driver_id();
	    format.bits        = 16;
	    format.channels    = 2;
	    format.rate        = samplerate;
	    format.byte_format = AO_FMT_LITTLE;
	    // -- Open driver --
	    device = ao_open_live(default_driver, &format, NULL );
	    if (device == NULL) { fprintf(stderr, "Error opening device.\n"); }

#endif
		// Adjust the audio lead time according to the audio_spec buffer size
		plm_set_audio_lead_time(self->plm, (double)4096/*audio_spec.samples*/
										 / (double)samplerate);
	}

	// Setup OpenGL shaders and textures
	const char * fsh = self->texture_mode == APP_TEXTURE_MODE_YCRCB
		? APP_FRAGMENT_SHADER_YCRCB
		: APP_FRAGMENT_SHADER_RGB;
	
	self->fragment_shader = app_compile_shader(self, GL_FRAGMENT_SHADER, fsh);
	self->vertex_shader   = app_compile_shader(self, GL_VERTEX_SHADER, APP_VERTEX_SHADER);
	self->shader_program  = glCreateProgram();

	glAttachShader(self->shader_program, self->vertex_shader);
	glAttachShader(self->shader_program, self->fragment_shader);
	glLinkProgram (self->shader_program);
	glUseProgram  (self->shader_program);
	
	// Create textures for YCrCb or RGB rendering
	if (self->texture_mode == APP_TEXTURE_MODE_YCRCB) {
		self->texture_y   = app_create_texture(self, 0, "texture_y" );
		self->texture_cb  = app_create_texture(self, 1, "texture_cb");
		self->texture_cr  = app_create_texture(self, 2, "texture_cr");
	}
	else {
		self->texture_rgb = app_create_texture(self, 0, "texture_rgb");
		int num_pixels    = plm_get_width(self->plm) * plm_get_height(self->plm);
		self->rgb_data    = (uint8_t*)malloc(num_pixels * 3);
	}
	
	return self;
}

void app_destroy(app_t *self)
{
	plm_destroy(self->plm);
	
	if (self->texture_mode == APP_TEXTURE_MODE_RGB) { free(self->rgb_data);	}

#ifdef _SDL_
	if (self->audio_device) { SDL_CloseAudioDevice(self->audio_device);	}
	SDL_GL_DeleteContext(self->gl);
	SDL_Quit();

#elif defined _LIBAO_
    // -- libao end --
    ao_close(device);
    ao_shutdown();

#endif
	free(self);
}


// video screen
static GLfloat model[] = // crops
{
    0,   0, // lower left
    0,   1, // upper left
    1,   0, // lower right
    1,   1  // upper right
};


double audio_lead = 0;

double last;
void app_update(app_t *self) {
#if defined _SDL_
	SDL_Event ev;
	while (SDL_PollEvent(&ev))
	{
		if (ev.type == SDL_QUIT
		|| (ev.type == SDL_KEYUP && ev.key.keysym.sym == SDLK_ESCAPE)
		) {	self->wants_to_quit = true;	}
		
		if (ev.type == SDL_WINDOWEVENT && ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
			glViewport(0, 0, ev.window.data1, ev.window.data2);
		}
	}
#endif
	// Compute the delta time since the last app_update(), limit max step to 
	// 1/30th of a second
	double current_time = (double)((unsigned int)get_time_ms()) / 1000.0;
	double elapsed_time = MIN(current_time - self->last_time, 1.0 / 30.0);
//printf("now: %f\t%f\t%f\t%f\n", current_time, elapsed_time, current_time - self->last_time);
	self->last_time = current_time;
	/*
	double now = (double)get_time_ms() / 1000.0;
	double elapsed_time2 = now - last;
printf("gtm: %f %f %f %f\n", now, now - last, last, elapsed_time2);
	last = now;
*/
	//printf("decode, elapsed_time: %f %lf %d\n", elapsed_time, (double)(get_time_ms()), SDL_GetTicks());

	// Decode
	plm_decode(self->plm, elapsed_time);
	
	if (plm_has_ended(self->plm)) {	self->wants_to_quit = true;	}
	
	//glClear(GL_COLOR_BUFFER_BIT);
#if 0
	//glRectf(0.0, 0.0, 1.0, 1.0);
//#else

//glUniform1i(glGetUniformLocation(self->shader_program, name), index);
GLuint position_loc = glGetAttribLocation( self->shader_program, "vertex" );
// glEnableClientState(GL_VERTEX_ARRAY);
//glEnableClientState(GL_TEXTURE_COORD_ARRAY);
// glVertexPointer  (2, GL_FLOAT, 0, model);
//glTexCoordPointer(2, GL_FLOAT, 0, texture);
glVertexAttribPointer     ( position_loc, 2, GL_FLOAT, false, 0, model );
   //glVertexAttribPointer ( vertex, 2, GL_FLOAT, false, 0, vertexArray );
glEnableVertexAttribArray ( position_loc );
   //glDrawArrays ( GL_TRIANGLE_STRIP, 0, 5 );
#endif

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	//we already swap!  // SDL_GL_SwapWindow(self->window);
}

GLuint app_compile_shader(app_t *self, GLenum type, const char *source) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		int log_written;
		char log[256];
		glGetShaderInfoLog(shader, 256, &log_written, log);
		printf("Error compiling shader: %s.\n", log);
	}
	printf("shader: %d\n", shader);
	return shader;
}

GLuint app_create_texture(app_t *self, GLuint index, const char *name) {
	GLuint texture;
	glCreateTextures(GL_TEXTURE_2D, 1, &texture);
	
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glUniform1i(glGetUniformLocation(self->shader_program, name), index);
	return texture;
}

void app_update_texture(app_t *self, GLuint unit, GLuint texture, plm_plane_t *plane) {
	glActiveTexture(unit);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_LUMINANCE, plane->width, plane->height, 0,
		GL_LUMINANCE, GL_UNSIGNED_BYTE, plane->data
	);
}

void app_on_video(plm_t *mpeg, plm_frame_t *frame, void *user) {
	app_t *self = (app_t *)user;
	
	// Hand the decoded data over to OpenGL. For the RGB texture mode, the
	// YCrCb->RGB conversion is done on the CPU.
	if (self->texture_mode == APP_TEXTURE_MODE_YCRCB) {
		app_update_texture(self, GL_TEXTURE0, self->texture_y,  &frame->y );
		app_update_texture(self, GL_TEXTURE1, self->texture_cb, &frame->cb);
		app_update_texture(self, GL_TEXTURE2, self->texture_cr, &frame->cr);
	}
	else {
		plm_frame_to_rgb(frame, self->rgb_data);
	
		glBindTexture(GL_TEXTURE_2D, self->texture_rgb);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGB, frame->width, frame->height, 0,
			GL_RGB, GL_UNSIGNED_BYTE, self->rgb_data
		);
	}
}

void app_on_audio(plm_t *mpeg, plm_samples_t *samples, void *user) {
	app_t *self = (app_t *)user;

	// Hand the decoded samples over to SDL or LIBAO
	
	int size = sizeof(float) * samples->count * 2;

#ifdef _SDL_
	SDL_QueueAudio(self->audio_device, samples->interleaved, size);
  //SDL_QueueAudio 2, 9216, 2304 (=1152*2ch samples!)

#elif defined _LIBAO_
	//printf("ao_play %d\n", size); 9216
	static short pBufferOut[1152*2*2];
	drmp3dec_f32_to_s16(samples->interleaved, &pBufferOut[0], 1152*2*2);    /* <-- Safe cast since pcmFramesJustRead will be clamped based on the size of tempF32 which is always small. */
        
    ao_play(device, (char *)&pBufferOut[0], 1152*2*2);
#endif
}



#ifndef PL_MPEG
int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Usage: pl_mpeg_player <file.mpg>\n");
		exit(1);
	}
	app_t *app = app_create(argv[1], APP_TEXTURE_MODE_YCRCB);

	while (!app->wants_to_quit) {
		app_update(app);
	}
	app_destroy(app);
#else

static app_t *app = NULL; // main object

int es2init_pl_mpeg(int window_width, int window_height)
{
	//printf("es2init_pl_mpeg\n");
	user_init(); // nfs

	printf("resolution: %dx%d\n", window_width, window_height);
	app = app_create("bjork-all-is-full-of-love.mpg", APP_TEXTURE_MODE_YCRCB);


	// gles2 attach vertexes
	GLuint position_loc = glGetAttribLocation( app->shader_program, "vertex" );

	glVertexAttribPointer     ( position_loc, 2, GL_FLOAT, false, 0, model );
	glEnableVertexAttribArray ( position_loc );

	glViewport(0, 0, window_width, window_height);
#endif
	
	return EXIT_SUCCESS;
}


void es2render_pl_mpeg(float dt)
{
	if(!app->wants_to_quit) { app_update(app); }
}


void es2end_pl_mpeg(void)
{
	user_end(); // nfs
}
