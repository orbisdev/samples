/*
    GLES2 texture from png using shaders and VBO
    2019, masterzorag
*/

#include <stdio.h>
#include <math.h>
#include <assert.h>

#include  <GLES2/gl2.h>
#include  <EGL/egl.h>

#include <debugnet.h>


GLuint simpleProgram;
static GLuint texture;
static GLuint buffer;
#define BUFFER_OFFSET(i) ((void*)(i))
 
static GLint a_position_location;
static GLint a_texture_coordinates_location;
static GLint u_texture_unit_location;
 
// position X, Y, texture S, T
static const float rect[] = {-1.0f, -1.0f, 0.0f, 0.0f,
                             -1.0f,  1.0f, 0.0f, 1.0f,
                              1.0f, -1.0f, 1.0f, 0.0f,
                              1.0f,  1.0f, 1.0f, 1.0f};
                              
GLuint create_vbo(const GLsizeiptr size, const GLvoid* data, const GLenum usage)
{
    assert(data != NULL);
    GLuint vbo_object;
    glGenBuffers(1, &vbo_object);
    assert(vbo_object != 0);
 
    glBindBuffer(GL_ARRAY_BUFFER, vbo_object);
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
 
    return vbo_object;
}

GLuint BuildShader(const char *source, GLenum shaderType)
{
	GLuint shaderHandle = glCreateShader(shaderType);
	glShaderSource(shaderHandle, 1, &source, 0);
	glCompileShader(shaderHandle);
	GLint compileSuccess;
	glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);

	if (compileSuccess == GL_FALSE)
	{
		GLchar messages[256];
		glGetShaderInfoLog(shaderHandle, sizeof(messages), 0, &messages[0]);
		debugNetPrintf(DEBUG, "compile glsl error : %s\n", messages);
	}

	return shaderHandle;
}

GLuint BuildProgram(const char *vShader, const char *fShader)
{
	GLuint vertexShader   = BuildShader(vShader, GL_VERTEX_SHADER);
	GLuint fragmentShader = BuildShader(fShader, GL_FRAGMENT_SHADER);

	GLuint programHandle = glCreateProgram();
	glAttachShader(programHandle, vertexShader);
	glAttachShader(programHandle, fragmentShader);
	glLinkProgram(programHandle);

	GLint linkSuccess;
	glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);
	if (linkSuccess == GL_FALSE)
	{
		GLchar messages[256];
		glGetProgramInfoLog(programHandle, sizeof(messages), 0, &messages[0]);
		debugNetPrintf(DEBUG, "compile glsl error : %s\n", messages);
	}

	return programHandle;
}

//https://github.com/learnopengles/airhockey/commit/228ce050da304258feca8d82690341cb50c27532
//OpenGLES2 handlers : init , final , update , render , touch-input
void on_GLES2_Init(int view_w, int view_h)
{
	const char *simpleVertexShader =
	   "attribute vec4 a_Position;"
       "attribute vec2 a_TextureCoordinates;"
       "varying   vec2 v_TextureCoordinates;"
       "void main()"
       "{"
       "v_TextureCoordinates = a_TextureCoordinates;"
       "gl_Position = a_Position;"
       "} ";
		
	const char *simpleFragmentShader =
	    "precision mediump float;"
	    "uniform   sampler2D u_TextureUnit;"
		"varying   vec2      v_TextureCoordinates;"
		"void main(void)"
		"{"
		" gl_FragColor = texture2D(u_TextureUnit, v_TextureCoordinates);"
		"}";

    texture = load_png_asset_into_texture("host0:tentacle.png");
    debugNetPrintf(DEBUG, "load_png_asset_into_texture ret: %d\n", texture);

    buffer = create_vbo(sizeof(rect), rect, GL_STATIC_DRAW);

	simpleProgram = BuildProgram(simpleVertexShader, simpleFragmentShader);
	glUseProgram(simpleProgram);
    
    a_position_location            = glGetAttribLocation (simpleProgram, "a_Position");
    a_texture_coordinates_location = glGetAttribLocation (simpleProgram, "a_TextureCoordinates");
    u_texture_unit_location        = glGetUniformLocation(simpleProgram, "u_TextureUnit");
}

void on_GLES2_Final()
{
	if (simpleProgram)
		glDeleteProgram(simpleProgram);

	simpleProgram = 0;
}

void on_GLES2_Size(int view_w, int view_h)
{
	glViewport(0, 0, view_w, view_h);
}

void on_GLES2_Update(float timeStep_sec)
{
    
}

void on_GLES2_Render()
{
    // we already clean in main renderloop()!
 
    glUseProgram(simpleProgram);
 
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(u_texture_unit_location, 0);
 
    glBindBuffer(GL_ARRAY_BUFFER, buffer); // bind VBO

    glVertexAttribPointer(a_position_location,
        2, GL_FLOAT, GL_FALSE, 4 * sizeof(GL_FLOAT), BUFFER_OFFSET(0));
    glVertexAttribPointer(a_texture_coordinates_location,
        2, GL_FLOAT, GL_FALSE, 4 * sizeof(GL_FLOAT), BUFFER_OFFSET(2 * sizeof(GL_FLOAT)));
    
    glEnableVertexAttribArray(a_position_location);
    glEnableVertexAttribArray(a_texture_coordinates_location);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
 
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // we already swapframe in main renderloop()!
}
