/*
    GLES2 texture from png using shaders and VBOs

    basic direct correlation: buffer[num] positions texture[num]

    2019, masterzorag
*/

#include <stdio.h>
#include <math.h>
#include <assert.h>
//#include <orbisGl.h>
//#include <debugnet.h>
#define  debugNetPrintf  fprintf
#define  DEBUG  stdout
//#define  
#include "defines.h"

#include "icons_shaders.h"
/*
    we setup two SL programs:
    0. use default color from texture
    1. use glowing effect on passed time
*/
#define NUM_OF_PROGRAMS  (2)


static GLuint slProgram[NUM_OF_PROGRAMS];
static GLuint simpleProgram;  // the current one

static GLuint texture[NUM_OF_TEXTURES];
static GLuint buffer [NUM_OF_TEXTURES];
#define BUFFER_OFFSET(i) ((void*)(i))


// the pngs we turn into VBOs + textures
char *pngs[NUM_OF_TEXTURES] =
{
    "/hostapp/system/textures/msxorbis.png", // fullscreen background
    "/hostapp/fames/game-icon.png",
    "/hostapp/fames/music-icon.png",
    "/hostapp/fames/themes-icon.png",
    "/hostapp/fames/settings-icon.png",
    "/hostapp/fames/emu-icon.png"
};

// the png we apply glowing effect by switching shader
int selected_icon = 1;

// shaders locations
static GLint a_position_location;
static GLint a_texture_coordinates_location;
static GLint u_texture_unit_location;
static GLint u_time_location;

// a fullscreen texture: position (X, Y), texture (S, T) (... or UVs)
static const float rect[] = { -1.0f, -1.0f, 0.0f, 1.0f,
                              -1.0f,  1.0f, 0.0f, 0.0f,
                               1.0f, -1.0f, 1.0f, 1.0f,
                               1.0f,  1.0f, 1.0f, 0.0f}; // not Y flipped!
static vec2 resolution;  // (constant)

static void setup_texture_position(int num, vec2 pos, const float scale_f)
{
    tex_size *= scale_f;

    /*p = pixel_to_normalized(pos, tex_size);*/

    vec4 p; // 2 points .xy pair: (x, y),  (x + texture.w, y + texture.h)

    p.xy  = -1. + 2. / resolution * pos; // (-1,-1) is BOTTOMLEFT, (1,1) is UPRIGHT
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

//https://github.com/learnopengles/airhockey/commit/228ce050da304258feca8d82690341cb50c27532
//OpenGLES2 handlers : init , final , update , render , touch-input
void on_GLES2_Init_icons(int view_w, int view_h)
{
    resolution = (vec2){ view_w, view_h }; // setup resolution for next setup_texture_position()

    for(int i = 0; i < NUM_OF_TEXTURES; i++)
    {
        texture[i] = load_png_asset_into_texture(pngs[i]);
        if(!texture[i])
            debugNetPrintf(DEBUG, "load_png_asset_into_texture '%s' ret: %d\n", pngs[i], texture[i]);

        // setup triangle positions to draw
        if(i)
            setup_texture_position( i, (vec2){ i *100, 480 /*ATTR_ORBISGL_HEIGHT*/ /2 }, 0.25 /* scale_f */);
        else
            buffer[i] = create_vbo(sizeof(rect), rect, GL_STATIC_DRAW); // background
    }

    for(int i = 0; i < NUM_OF_PROGRAMS; i++)
    {
        slProgram[i] =
        #ifdef HAVE_SHACC
            BuildProgram(simpleVertexShader, simpleFragmentShader[i]);
        #else
            CreateProgramFromBinary(i);
        #endif
        debugNetPrintf(DEBUG, "slProgram[%d]: %u\n", i, slProgram[i]);
        simpleProgram = slProgram[i];

        glUseProgram(simpleProgram);
        a_position_location            = glGetAttribLocation (simpleProgram, "a_Position");
        a_texture_coordinates_location = glGetAttribLocation (simpleProgram, "a_TextureCoordinates");
        u_texture_unit_location        = glGetUniformLocation(simpleProgram, "u_TextureUnit");
        u_time_location                = glGetUniformLocation(simpleProgram, "u_time");
    }
    // reset to first one
    simpleProgram = slProgram[0];
}

void on_GLES2_Final(void)
{
    for(int i = 0; i < NUM_OF_PROGRAMS; ++i)
        { if(slProgram[i]) glDeleteProgram(slProgram[i]), slProgram[i] = 0; }
}

void on_GLES2_Size_icons(int view_w, int view_h)
{
    glViewport(0, 0, view_w, view_h);
}


void on_GLES2_Update(int frame)
{
    float t = (float)frame /10.; // slow down

    for(int i = 0; i < NUM_OF_PROGRAMS; ++i)
    {
        glUseProgram(slProgram[i]);
        // write the value to the shaders
        printf("%.4f %d\n", t, frame);
        glUniform1f(glGetUniformLocation(slProgram[i], "u_time"), t);
    }
}

void on_GLES2_Render(int num) // which texture to draw
{
    // we already clean

    simpleProgram = slProgram[0]; // default

    if(num == selected_icon) simpleProgram = slProgram[1]; // glowing effect

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

    // release VBO, texture and program
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glActiveTexture(0); // error on piglet !!
    glUseProgram(0);

    // we already flip/swap
}

