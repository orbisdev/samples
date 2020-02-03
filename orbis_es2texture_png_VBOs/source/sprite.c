/*
    GLES2 texture from png using shaders and VBO

    spritessheet concept, one texture - one VBO;
    address sprite by UVs offset in shader code

    based on the texture rectangle sample from:
    https://www.raylib.com/examples/web/textures/loader.html?name=textures_rectangle

    2019, masterzorag
*/

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <orbisGl.h>
#include <debugnet.h>

#include "defines.h"

/*
    we setup one SL program:
    - use default color from texture
    - apply UVs offset on passed num
*/
#define NUM_OF_SPRITES  (6)


static GLuint simpleProgram;


static GLuint texture;
static GLuint buffer;
#define BUFFER_OFFSET(i) ((void*)(i))


// the png we turn into VBO + texture
char *png =
{
/*
   [Scarfy spritesheet](scarfy.png) by [Eiden Marsal]
   (https://www.artstation.com/artist/marshall_z),
   licensed as [Creative Commons Attribution-NonCommercial 3.0]
   (https://creativecommons.org/licenses/by-nc/3.0/legalcode)
*/
    "host0:scarfy.png"
    //"host0:tentacle.png", // fullscreen background
    //"/app0/sce_sys/pic0.png", // OL fullscreen background
    //"host0:sce_sys/pic0.png",
};

// shaders locations
static GLint a_position_location;
static GLint a_texture_coordinates_location;
static GLint u_texture_unit_location;
static GLint u_offset_location; // to address each sprite in texture displacing UVs
static GLint u_p1_pos_location; // to offset position

static vec2 resolution;  // (constant)

static void setup_sprite_position(int num, vec2 pos, const float scale_f)
{
    tex_size   *= scale_f;
    tex_size.x /= NUM_OF_SPRITES; // split texture horizontally

    /*p = pixel_to_normalized(pos, tex_size);*/

    vec4 p; // 2 points .xy pair: (x, y),  (x + texture.w, y + texture.h)

    p.xy  = -1. + 2. / resolution * pos; // (-1,-1) is BOTTOMLEFT, (1,1) is UPRIGHT
    p.zw  = -1. + 2. / resolution * (pos + tex_size);
    p.yw *= -1.; // flip Y axis

    /* setup VBO from texture position, use
       orig_texture_size_in_px * scale_f for rendered size,
       but render just one of NUM_OF_SPRITES width */
    const float vertexes[] = { p.x, p.y,  0.f,                 0.f,   // TPLF
                               p.x, p.w,  0.f,                 1.f,   // BTLF
                               p.z, p.y,  1.f /NUM_OF_SPRITES, 0.f,   // BTRG
                               p.z, p.w,  1.f /NUM_OF_SPRITES, 1.f }; // TPRG
    // setup triangle positions to draw
    buffer = create_vbo(sizeof(vertexes), vertexes, GL_STATIC_DRAW);
}


//https://github.com/learnopengles/airhockey/commit/228ce050da304258feca8d82690341cb50c27532
//OpenGLES2 handlers : init , final , update , render , touch-input
void on_GLES2_Init_sprite(int view_w, int view_h)
{
    resolution = (vec2){ view_w, view_h }; // setup resolution for next setup_texture_position()

    texture = load_png_asset_into_texture(png);
    if(!texture)
        debugNetPrintf(ERROR, "load_png_asset_into_texture '%s' ret: %d\n", png, texture);

    /// setup triangle positions to draw
    setup_sprite_position( 0, (vec2){ 0, 0 }, 1.f /* scale_f */);

    #ifdef HAVE_SHACC
        simpleProgram = BuildProgram(vs, fs);
    #else
        simpleProgram = CreateProgramFromBinary(2); // todo: enums for "the Nth SL program"
    #endif
    debugNetPrintf(DEBUG, "simpleProgram: %u\n", simpleProgram);

    glUseProgram(simpleProgram);
    a_position_location            = glGetAttribLocation (simpleProgram, "a_Position");
    a_texture_coordinates_location = glGetAttribLocation (simpleProgram, "a_TextureCoordinates");
    u_texture_unit_location        = glGetUniformLocation(simpleProgram, "u_TextureUnit");
    u_offset_location              = glGetUniformLocation(simpleProgram, "u_offset");
    u_p1_pos_location              = glGetUniformLocation(simpleProgram, "u_p1_pos");
}

void on_GLES2_Final_sprite(void)
{
    if(simpleProgram) glDeleteProgram(simpleProgram), simpleProgram = 0;
}

void on_GLES2_Size_sprite(int view_w, int view_h)
{
    glViewport(0, 0, view_w, view_h);
}

static vec2 uv     = { 0.f };
       vec2 p1_pos = { 0.f };

void on_GLES2_Update_sprite(int frame)
{
    #define FRAME_NUM  (10)
    if(!(frame %FRAME_NUM)) // update UVs
    {
        uv = (vec2){ 1. /NUM_OF_SPRITES * ((frame /FRAME_NUM) %NUM_OF_SPRITES), 0. };
      /*debugNetPrintf(DEBUG, "frame:%d sprite:%d uv(%.3f %.3f)\n",
                                    frame, (frame /FRAME_NUM) %NUM_OF_SPRITES, uv.x, uv.y);*/
    }
    glUseProgram(simpleProgram);
    // write the values to the shaders
    glUniform2f(u_offset_location, uv.x,     uv.y     );
    glUniform2f(u_p1_pos_location, p1_pos.x, p1_pos.y );
}

void on_GLES2_Render_sprite(int frame) // which sprite to draw (?!)
{
    // we already clean

    glUseProgram(simpleProgram);

    glDisable(GL_CULL_FACE);

    // enable alpha for png textures
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // select requested texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(u_texture_unit_location, 0); // tell to shader
    glBindBuffer(GL_ARRAY_BUFFER, buffer);   // bind requested VBO

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
