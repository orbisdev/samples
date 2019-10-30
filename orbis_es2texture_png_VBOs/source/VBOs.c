/*
    GLES2 texture from png using shaders and VBOs
*/

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <orbisGl.h>
#include <debugnet.h>

#include "defines.h"


static GLuint simpleProgram;
static GLuint texture[NUM_OF_TEXTURES];
static GLuint buffer [NUM_OF_TEXTURES];
#define BUFFER_OFFSET(i) ((void*)(i))


// the pngs we turn into VBOs + textures
char *pngs[NUM_OF_TEXTURES] =
{
    "host0:tentacle.png", // fullscreen background
    "host0:fames/game-icon.png",
    "host0:fames/music-icon.png",
    "host0:fames/themes-icon.png",
    "host0:fames/settings-icon.png",
    "host0:fames/emu-icon.png"
};

static GLint a_position_location;
static GLint a_texture_coordinates_location;
static GLint u_texture_unit_location;

// a fullscreen texture: position (X, Y), texture (S, T) (... or UVs)
static const float rect[] = { -1.0f, -1.0f, 0.0f, 0.0f,
                              -1.0f,  1.0f, 0.0f, 1.0f,
                               1.0f, -1.0f, 1.0f, 0.0f,
                               1.0f,  1.0f, 1.0f, 1.0f};

static vec2 resolution;  // (constant)
       vec2 tex_size;    // last loaded png size as (w, h)

static void setup_texture_position(int num, vec2 pos, const float scale_f)
{
    tex_size *= scale_f;

    /*p = pixel_to_normalized(pos, tex_size);*/

    vec4 p; // (x, y),  (x + texture.w, y + texture.h)

    p.xy  = -1. + 2. / resolution * pos; // (-1,-1) is BOTTOMLEFT, (1,1) IS UPRIGHT
    p.zw  = -1. + 2. / resolution * (pos + tex_size);
    p.yw *= -1.; // flip Y axis

    /* setup VBO from texture position, use
       orig_texture_size_in_px * scale_f for rendered size */
    const float vertexes[] = { p.x, p.y,  0.f, 0.f,   // TPLF
                               p.x, p.w,  0.f, 1.f,   // BTLF
                               p.z, p.y,  1.f, 0.f,   // BTRG
                               p.z, p.w,  1.f, 1.f }; // TPRG

    // setup triangle positions to draw
    buffer[num] = create_vbo(sizeof(vertexes), vertexes, GL_STATIC_DRAW);
}

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
       "attribute vec4 a_Position; \
        attribute vec2 a_TextureCoordinates; \
        varying   vec2 v_TextureCoordinates; \
        void main() \
        { \
          v_TextureCoordinates = a_TextureCoordinates; \
          gl_Position = a_Position; \
        }";

     const char *simpleFragmentShader =
       "precision mediump float; \
        uniform   sampler2D u_TextureUnit; \
        varying   vec2      v_TextureCoordinates; \
        void main(void) \
        { \
          gl_FragColor = texture2D(u_TextureUnit, v_TextureCoordinates); \
        }";

    resolution = (vec2){ view_w, view_h }; // setup resolution for next setup_texture_position()

    for(int i = 0; i < NUM_OF_TEXTURES; i++)
    {
        texture[i] = load_png_asset_into_texture(pngs[i]);
        debugNetPrintf(DEBUG, "load_png_asset_into_texture '%s' ret: %d\n", pngs[i], texture[i]);

        // setup triangle positions to draw
        if(i)
            setup_texture_position( i, (vec2){ i *100, ATTR_ORBISGL_HEIGHT /2 }, 0.25 /* scale_f */);
        else
            buffer[i] = create_vbo(sizeof(rect), rect, GL_STATIC_DRAW); // background
    }

    simpleProgram = BuildProgram(simpleVertexShader, simpleFragmentShader);
    glUseProgram(simpleProgram);

    a_position_location            = glGetAttribLocation (simpleProgram, "a_Position");
    a_texture_coordinates_location = glGetAttribLocation (simpleProgram, "a_TextureCoordinates");
    u_texture_unit_location        = glGetUniformLocation(simpleProgram, "u_TextureUnit");
}

void on_GLES2_Final(void)
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


void on_GLES2_Render(int num) // which texture to draw
{
    // we already clean

    glUseProgram(simpleProgram);

    glDisable(GL_CULL_FACE);

    // enable alpha for png textures
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // select requested texture
    glActiveTexture(GL_TEXTURE0 + num);
    glBindTexture(GL_TEXTURE_2D, texture[num]);
    glUniform1i(u_texture_unit_location, num);  // tell to shader
    glBindBuffer(GL_ARRAY_BUFFER, buffer[num]); // bind requested VBO

    // setup attr
    glVertexAttribPointer(a_position_location,
        2, GL_FLOAT, GL_FALSE, 4 * sizeof(GL_FLOAT), BUFFER_OFFSET(0));
    glVertexAttribPointer(a_texture_coordinates_location,
        2, GL_FLOAT, GL_FALSE, 4 * sizeof(GL_FLOAT), BUFFER_OFFSET(2 * sizeof(GL_FLOAT)));

    // pin variables
    glEnableVertexAttribArray(a_position_location);
    glEnableVertexAttribArray(a_texture_coordinates_location);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // draw binded VBO buffer

    // revert state back
    glDisable(GL_BLEND);

    // release VBO and texture
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // we already flip/swap
}
