//Authors : Lee Jeong Seop
//CC BY-NC-SA 4.0

#if CCR_FORCE_LLVM_INTERPRETER
#error "Clang/LLVM interpreter does not support native to script callback function yet. Consider using CPP built-in compiler or turn on 'Use JIT execution' compiler option in app options menu."
#endif

// std
#include <stdio.h>
#include <math.h>

// OpenGL
#include  <GLES2/gl2.h>
#include  <EGL/egl.h>

//------------------------------------------------------------------------------

#define SAMPLE_POINT_COUNT 200

GLuint simpleProgram;

struct Vertex
{
	float Position[2];
	float Color[4];
};

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
	GLint compileSuccess;
	glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);

	if (compileSuccess == GL_FALSE)
	{
		GLchar messages[256];
		glGetShaderInfoLog(shaderHandle, sizeof(messages), 0, &messages[0]);
		printf("compile glsl error : %s\n", messages);
	}

	return shaderHandle;
}
//------------------------------------------------------------------------------
GLuint BuildProgram(const char *vShader, const char *fShader)
{
	GLuint vertexShader = BuildShader(vShader, GL_VERTEX_SHADER);
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
		printf("compile glsl error : %s\n", messages);
	}

	return programHandle;
}
//------------------------------------------------------------------------------
void on_GLES2_Size(int view_w, int view_h)
{
	glViewport(0, 0, view_w, view_h);
	ApplyOrtho(1, 1);
}


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
{}
//------------------------------------------------------------------------------
void on_GLES2_Render()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1);
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

	struct Vertex vertices[SAMPLE_POINT_COUNT];
	for (int i = 0; i < SAMPLE_POINT_COUNT; i++)
	{
		float x = -1.0f + (i / (float)(SAMPLE_POINT_COUNT - 1))*2.0f;
		vertices[i].Position[0] = x;//x
		vertices[i].Position[1] = sinf(x * 20) / 3 + sinf(x * 40) / 4 + sinf(x * 60) / 5;//y

		vertices[i].Color[0] = 0.0f;//r
		vertices[i].Color[1] = 1.0f;//g
		vertices[i].Color[2] = 0.0f;//b
		vertices[i].Color[3] = 1.0f;//a
	}

	GLsizei stride = sizeof(struct Vertex);
	const GLvoid *pCoords = &vertices[0].Position[0];
	const GLvoid *pColors = &vertices[0].Color[0];

	glVertexAttribPointer(positionSlot, 2, GL_FLOAT, GL_FALSE, stride, pCoords);
	glVertexAttribPointer(colorSlot, 4, GL_FLOAT, GL_FALSE, stride, pColors);

	GLsizei vertexCount = sizeof(vertices) / sizeof(struct Vertex);
	glDrawArrays(GL_LINE_STRIP, 0, vertexCount);

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

