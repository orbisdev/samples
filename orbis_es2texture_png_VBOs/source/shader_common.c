/*
    shader_common.c

    shaders facilities about building and loading shaders
*/

#include <stdio.h>
#include <math.h>
#include <assert.h>

#include <orbisGl.h>
#include <debugnet.h>

#include "defines.h"


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

static GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vertexShader);
    glAttachShader(programHandle, fragmentShader);
    glLinkProgram (programHandle);

    GLint linkSuccess;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);
    if (linkSuccess == GL_FALSE)
        { debugNetPrintf(ERROR, "GL_LINK_STATUS error\n"); }

    if(vertexShader)   { glDeleteShader(vertexShader),   vertexShader   = 0; }
    if(fragmentShader) { glDeleteShader(fragmentShader), fragmentShader = 0; }

    return programHandle;
}

#ifdef HAVE_SHACC

static GLuint BuildShader(const char *source, GLenum shaderType)
{
    GLuint shaderHandle = glCreateShader(shaderType);

    glShaderSource (shaderHandle, 1, &source, 0);
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
    /* after built, dump those in .sb form
    DumpShader(vertexShader,   "vShader");
    DumpShader(fragmentShader, "fShader");
    */
    return LinkProgram(vertexShader, fragmentShader);
}

#else // HAVE_SHACC

GLuint CreateProgramFromBinary(unsigned int i) //const char *vShader, const char *fShader)
{
    GLuint vertexShader   = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint programHandle  = 0;
    int ret;

    if(!vertexShader)
        { ret = glGetError(); debugNetPrintf(ERROR, "glCreateShader(%d)", ret);  goto err; }
    else
    if(!fragmentShader)
        { ret = glGetError(); debugNetPrintf(ERROR, "glCreateShader(%d)", ret);  goto err; }

    const GLvoid *fShader; // use embedded .sb
    switch(i)
    {   // select which fragment shader will be used, for program i //todo enums
        default:
        /* icons_binary_shaders.h */
        case 0:  fShader = &fShader0; break;
        case 1:  fShader = &fShader1; break;
        /* more SL program pairs to add ! */
    }
    glShaderBinary( 1, &vertexShader,   2, (const GLvoid*)&vShader, vShader_len[0]);
    glShaderBinary( 1, &fragmentShader, 2,                 fShader, fShader_len[i]);

    programHandle = LinkProgram(vertexShader, fragmentShader);
err:
    return programHandle;
}

#endif // HAVE_SHACC
