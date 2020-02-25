/*
    GLES2 texture from png using shaders and VBO

    spritessheet concept, one texture - one VBO;
    address sprite by UVs offset in shader code

    2019, masterzorag
*/

#include <stdio.h>
#include <math.h>
#include <assert.h>
//#include <orbisGl.h>
//#include <debugnet.h>
#include  <GLES2/gl2.h>
#include  <EGL/egl.h>


#include "defines.h" // tex_size

/*
    we setup 1 SL program:
    use default color from texture
    apply UVs offset on passed num
*/
#if 1 // HAVE_SHACC
    #include "sprite_shaders.h"
#else
    #include "sprite_shaders.sb.bin"
#endif

static GLuint simpleProgram;


static GLuint texture;
static GLuint buffer;
#define BUFFER_OFFSET(i) ((void*)(i))


// the png we turn into VBO + texture
char *png =
{
    "scarfy.png"
    //"/Archive/PS4-work/OrbisLink/ps4sh/bin/tentacle.png", // fullscreen background
    //"host0:tentacle.png", // fullscreen background
    //"/mnt/usb0/mz.png",//log: [PS4][DEBUG]: [ORBISFILE] Failed to read file /mnt/usb0/mz.png
    //"/app0/sce_sys/pic0.png", // OL fullscreen background
    //"host0:sce_sys/pic0.png",
    //"host0:mz2.png",
};

// shaders locations
static GLint a_position_location;
static GLint a_texture_coordinates_location;
static GLint u_texture_unit_location;
static GLint u_offset_location; // to address each sprite in texture displacing UVs
static GLint u_p1_pos_location; // to change VBO position

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
    const float vertexes[] = { // facing right
                               p.x, p.y,  0.f,                 0.f,   // TPLF
                               p.x, p.w,  0.f,                 1.f,   // BTLF
                               p.z, p.y,  1.f /NUM_OF_SPRITES, 0.f,   // BTRG
                               p.z, p.w,  1.f /NUM_OF_SPRITES, 1.f,   // TPRG
                               // facing left
                               p.x, p.y,  1.f /NUM_OF_SPRITES, 0.f,   // TPLF
                               p.x, p.w,  1.f /NUM_OF_SPRITES, 1.f,   // BTLF
                               p.z, p.y,  0.f,                 0.f,   // BTRG
                               p.z, p.w,  0.f,                 1.f }; // TPRG
    // setup triangle positions to draw
    buffer = create_vbo(sizeof(vertexes), vertexes, GL_STATIC_DRAW);

/*  printf("%4d,%4d ->\t%3.4f, %3.4f\n",
         (int)pos.x,                (int)pos.y,                 p.x, p.y );
    printf("%4d,%4d ->\t%3.4f, %3.4f\n",
         (int)(pos.x + tex_size.x), (int)(pos.y + tex_size.y),  p.z, p.w ); */
}


//https://github.com/learnopengles/airhockey/commit/228ce050da304258feca8d82690341cb50c27532
//OpenGLES2 handlers : init , final , update , render , touch-input
void on_GLES2_Init_sprite(int view_w, int view_h)
{
    resolution = (vec2){ view_w, view_h }; // setup resolution for next setup_texture_position()

    texture = load_png_asset_into_texture(png);
    if(!texture)
        printf("load_png_asset_into_texture '%s' ret: %d\n", png, texture);

    /// setup triangle positions to draw
    setup_sprite_position( 0, (vec2){ 0, 0 }, 1.f /* scale_f */);

    #if 1 // HAVE_SHACC
      simpleProgram = BuildProgram(vs, fs);
    #else
      simpleProgram = CreateProgramFromBinary("host0:compiled/vert.sb", "host0:compiled/frag.sb");
    #endif
    printf("simpleProgram: %u\n", simpleProgram);

    glUseProgram(simpleProgram);
    a_position_location            = glGetAttribLocation (simpleProgram, "a_Position");
    a_texture_coordinates_location = glGetAttribLocation (simpleProgram, "a_TextureCoordinates");
    u_texture_unit_location        = glGetUniformLocation(simpleProgram, "u_TextureUnit");
    u_offset_location              = glGetUniformLocation(simpleProgram, "u_offset");
    u_p1_pos_location              = glGetUniformLocation(simpleProgram, "u_p1_pos");
}

void on_GLES2_Final_sprite(void)
{
    if (simpleProgram) glDeleteProgram(simpleProgram), simpleProgram = 0;
    if (texture)       glDeleteTextures(1, &texture),  texture = 0;
}

void on_GLES2_Size_sprite(int view_w, int view_h)
{
    glViewport(0, 0, view_w, view_h);
}

extern float p1_pos_x, p1_pos_y; // from main(), by mouse
void on_GLES2_Update_sprite(int frame)
{

    int num = frame %NUM_OF_SPRITES;
    vec2 uv = (vec2){ 1. /NUM_OF_SPRITES * num, 0. };
    glUseProgram(simpleProgram);
    // write the values to the shaders
    glUniform2f(u_offset_location, uv.x, uv.y );
    glUniform2f(u_p1_pos_location, p1_pos_x, p1_pos_y );
    //printf("%d %f %f\n", num, uv.x, uv.y);
}

int is_facing_left = 0;
void on_GLES2_Render_sprite(int num) // which texture to draw
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
        2, GL_FLOAT, GL_FALSE, 4 * sizeof(GL_FLOAT), BUFFER_OFFSET(( 2 + 16 * is_facing_left )
                                                                   * sizeof( GL_FLOAT )) );
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

