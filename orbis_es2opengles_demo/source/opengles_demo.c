//Original source code found at : book, iPhone 3D Programming 2010 by O'Reilly Media, Inc.

// supported platforms check. NOTE iOS, Android and Windows only, but may works on other platforms ;-)

#ifdef CCR_FORCE_LLVM_INTERPRETER
	#error "Clang/LLVM on iOS does not support function pointer yet. Consider using CPP built-in compiler."
#endif

// std
#include <stdio.h>

// OpenGL
#include  <GLES2/gl2.h>
#include  <EGL/egl.h>

//------------------------------------------------------------------------------

GLuint simpleProgram;
//------------------------------------------------------------------------------
struct Vertex
{
	float Position[2];
	float Color[4];
};
//------------------------------------------------------------------------------
const struct Vertex Vertices[6] =
{
	{{-0.5,-0.866},{1,1,0.5f,1}},
	{{0.5,-0.866},{1,1,0.5f,1}},
	{{0,1},{1,1,0.5f,1}},
	{{-0.5,-0.866},{0.7f,0.7f,0.7f,1}},
	{{0.5,-0.866},{0.7f,0.7f,0.7f,1}},
	{{0,-0.4f},{0.7f,0.7f,0.7f,1}},
};
//------------------------------------------------------------------------------
void ApplyOrtho(float maxX, float maxY)
{
	float a = 1.0f / maxX;
	float b = 1.0f / maxY;
	float ortho[16] =
	{
		a,0,0,0,
		0,b,0,0,
		0,0,-1,0,
		0,0,0,1
	};

	GLint projectionUniform = glGetUniformLocation(simpleProgram, "Projection");
	glUniformMatrix4fv(projectionUniform, 1, 0, &ortho[0]);
}
//------------------------------------------------------------------------------
GLuint BuildShader(const char *source, GLenum shaderType)
{
	GLuint shaderHandle = glCreateShader(shaderType);
	glShaderSource(shaderHandle, 1, &source, 0);
	glCompileShader(shaderHandle);

	// get compiler result
	GLint compileSuccess;
	glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);

	// succeeded?
	if (compileSuccess == GL_FALSE)
	{
		// show error message (in console)
		GLchar messages[256];
		glGetShaderInfoLog(shaderHandle, sizeof(messages), 0, &messages[0]);
		printf("compile glsl error : %s\n", messages);
	}

	return shaderHandle;
}
//------------------------------------------------------------------------------
GLuint BuildProgram(const char *vShader, const char *fShader)
{
	// create and compile vertex and fragment shaders programs
	GLuint vertexShader = BuildShader(vShader, GL_VERTEX_SHADER);
	GLuint fragmentShader = BuildShader(fShader, GL_FRAGMENT_SHADER);

	// link shader programs
	GLuint programHandle = glCreateProgram();
	glAttachShader(programHandle, vertexShader);
	glAttachShader(programHandle, fragmentShader);
	glLinkProgram(programHandle);

	// get linker result
	GLint linkSuccess;
	glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);
	// succeeded?
	if (linkSuccess == GL_FALSE)
	{
		// show error message (in console)
		GLchar messages[256];
		glGetProgramInfoLog(programHandle, sizeof(messages), 0, &messages[0]);
		printf("compile glsl error : %s\n", messages);
	}

	return programHandle;
}
//------------------------------------------------------------------------------
void on_GLES2_Size(int view_w, int view_h)
{
	glViewport(0, 0, view_w, view_h);
	ApplyOrtho(2, 3);
}
//------------------------------------------------------------------------------
//OpenGLES2 handlers : init , final , update , render , touch-input
void on_GLES2_Init(int view_w, int view_h)
{
	const char *simpleVertexShader =
		"attribute vec4 Position;"
		"attribute vec4 SourceColor;"
		"varying vec4 DestinationColor;"
		"uniform mat4 Modelview;"
		"uniform mat4 Projection;"
		"void main(void)"
		"{"
		" DestinationColor=SourceColor;"
		" gl_Position=Projection*Modelview*Position;"
		"}";

	const char *simpleFragmentShader =
		"varying lowp vec4 DestinationColor;"
		"void main(void)"
		"{"
		" gl_FragColor=DestinationColor;"
		"}";

	simpleProgram = BuildProgram(simpleVertexShader, simpleFragmentShader);
	glUseProgram(simpleProgram);

	// set viewport
	on_GLES2_Size(view_w, view_h);
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
	if (simpleProgram)
		glDeleteProgram(simpleProgram);
	simpleProgram = 0;
}


//------------------------------------------------------------------------------
void on_GLES2_Update(float timeStep_sec)
{
}

//------------------------------------------------------------------------------
void on_GLES2_Render()
{
	// clear scene background and depth buffer
	glClearColor(0.7f, 0.7f, 0.7f, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	float modelViewMatrix[16] =
	{
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};
	GLint modelviewUniform = glGetUniformLocation(simpleProgram, "Modelview");
	glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix[0]);

	GLuint positionSlot = glGetAttribLocation(simpleProgram, "Position");
	GLuint colorSlot = glGetAttribLocation(simpleProgram, "SourceColor");

	glEnableVertexAttribArray(positionSlot);
	glEnableVertexAttribArray(colorSlot);

	GLsizei stride = sizeof(struct Vertex);
	const GLvoid *pCoords = &Vertices[0].Position[0];
	const GLvoid *pColors = &Vertices[0].Color[0];

	glVertexAttribPointer(positionSlot, 2, GL_FLOAT, GL_FALSE, stride, pCoords);
	glVertexAttribPointer(colorSlot, 4, GL_FLOAT, GL_FALSE, stride, pColors);

	GLsizei vertexCount = sizeof(Vertices) / sizeof(struct Vertex);
	glDrawArrays(GL_TRIANGLES, 0, vertexCount);

	// disconnect slots from shader
	glDisableVertexAttribArray(positionSlot);
	glDisableVertexAttribArray(colorSlot);
}
//------------------------------------------------------------------------------
void on_GLES2_TouchBegin(float x, float y)
{
	printf("on_GLES2_TouchBegin(x=%f,y=%f);\n", x, y);
}
//------------------------------------------------------------------------------
void on_GLES2_TouchEnd(float x, float y)
{
	printf("on_GLES2_TouchEnd(x=%f,y=%f);\n", x, y);
}
//------------------------------------------------------------------------------
void on_GLES2_TouchMove(float prev_x, float prev_y, float x, float y)
{
	printf("on_GLES2_TouchMove(prev_x=%f,prev_y=%f,x=%f,y=%f);\n",
		prev_x, prev_y, x, y);
}

