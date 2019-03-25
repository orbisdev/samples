/*
code build and run on linux or ps4, following __PS4__ definition:
liborbisGl, libfreetype and libfreetype-gl are required
on pc runs with GLUT

clang \
mat4.c \
vector.c \
texture-font.c \
texture-atlas.c \
vertex-attribute.c \
vertex-buffer.c \
demo-font.c \
-lGL -lm -lX11 -lEGL  -I$PS4SDK/include/freetype2 -lfreetype -lglut -v -o demo-font

 clang source/ *.c -lGL -lm -lX11 -lEGL  -I$PS4SDK/include/freetype2 -lfreetype -lglut -o demo-font
*/

/* ============================================================================
 * Freetype GL - A C OpenGL Freetype engine
 * Platform:    Any
 * WWW:         http://code.google.com/p/freetype-gl/
 * ----------------------------------------------------------------------------
 * Copyright 2011,2012 Nicolas P. Rougier. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NICOLAS P. ROUGIER ''AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL NICOLAS P. ROUGIER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Nicolas P. Rougier.
 * ============================================================================
 *
 * Example showing regular font usage
 *
 * ============================================================================
 */
#include <stdio.h>
#include <wchar.h>

#include <freetype-gl.h>  // links against libfreetype-gl

#if defined(__APPLE__)
    #include <Glut/glut.h>
#elif defined(_WIN32) || defined(_WIN64)
    #include <GLUT/glut.h>
#elif defined(__PS4__)
    #include <debugnet.h>
    #include <orbisFile.h>
#else
    #include <GL/glut.h>
#endif


//missing guards
//#include "MiniShader.h"  // miniCompileShaders

// ------------------------------------------------------- typedef & struct ---
typedef struct {
    float x, y, z;    // position
    float s, t;       // texture
    float r, g, b, a; // color
} vertex_t;


// ------------------------------------------------------- global variables ---
GLuint shader;
vertex_buffer_t *buffer;
mat4   model, view, projection;
texture_atlas_t *atlas;

// ---------------------------------------------------------------- reshape ---
void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    mat4_set_orthographic( &projection, 0, width, 0, height, -1, 1);
}

// ---------------------------------------------------------------- display ---
void render_text( void )
{
    // we already clean in main renderloop()!

    glUseProgram( shader );
    glBindTexture( GL_TEXTURE_2D, atlas->id );  // rebind

    glDisable(GL_CULL_FACE);  // just in case, disable it

    // freetype-gl shaders wanted config
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    //glUseProgram( shader );
    {
        glUniform1i(        glGetUniformLocation( shader, "texture" ),    0 );
        glUniformMatrix4fv( glGetUniformLocation( shader, "model" ),      1, 0, model.data);
        glUniformMatrix4fv( glGetUniformLocation( shader, "view" ),       1, 0, view.data);
        glUniformMatrix4fv( glGetUniformLocation( shader, "projection" ), 1, 0, projection.data);
        vertex_buffer_render( buffer, GL_TRIANGLES );
    }

    glDisable( GL_BLEND );  // Reset state back

    // we already swapframe in main renderloop()!
}


// --------------------------------------------------------------- add_text ---
void add_text( vertex_buffer_t * buffer, texture_font_t * font,
               wchar_t * text, vec4 * color, vec2 * pen )
{
    size_t i;
    float r = color->red, g = color->green, b = color->blue, a = color->alpha;
    for( i=0; i<wcslen(text); ++i )
    {
        texture_glyph_t *glyph = texture_font_get_glyph( font, text[i] );
        if( glyph != NULL )
        {
            int kerning = 0;
            if( i > 0)
            {
                kerning = texture_glyph_get_kerning( glyph, text[i-1] );
            }
            pen->x += kerning;
            int x0  = (int)( pen->x + glyph->offset_x );
            int y0  = (int)( pen->y + glyph->offset_y );
            int x1  = (int)( x0 + glyph->width );
            int y1  = (int)( y0 - glyph->height );
            float s0 = glyph->s0;
            float t0 = glyph->t0;
            float s1 = glyph->s1;
            float t1 = glyph->t1;
            GLuint indices[6] = {0,1,2, 0,2,3};
            vertex_t vertices[4] = { { x0,y0,0,  s0,t0,  r,g,b,a },
                                     { x0,y1,0,  s0,t1,  r,g,b,a },
                                     { x1,y1,0,  s1,t1,  r,g,b,a },
                                     { x1,y0,0,  s1,t0,  r,g,b,a } };
            vertex_buffer_push_back( buffer, vertices, 4, indices, 6 );
            pen->x += glyph->advance_x;
        }
    }
}
// ------------------------------------------------------ freetype-gl shaders ---
GLuint CreateProgram(void)
{
    GLuint programID = 0;

    const GLchar s_vertex_shader_code[] =
       "precision mediump float; \
        uniform mat4 model; \
        uniform mat4 view; \
        uniform mat4 projection; \
        \
        attribute vec3 vertex; \
        attribute vec2 tex_coord; \
        attribute vec4 color; \
        \
        varying vec2 vTexCoord; \
        varying vec4 fragColor; \
        \
        void main() \
        { \
            vTexCoord.xy = tex_coord.xy; \
            fragColor    = color; \
            gl_Position  = projection*(view*(model*vec4(vertex,1.0))); \
        }";

    const GLchar s_fragment_shader_code[] =
       "precision mediump float; \
        uniform sampler2D texture; \
        \
        varying vec2 vTexCoord; \
        varying vec4 fragColor; \
        \
        void main() \
        { \
            float a = texture2D(texture, vTexCoord).a; \
            gl_FragColor = vec4(fragColor.rgb, fragColor.a*a); \
        }";

    // we can use OrbisGl wrappers, or MiniAPI ones
//#define __PGL_ORBISGL__

#if defined __PGL_ORBISGL__
    GLuint vertexShader;
    GLuint fragmentShader;

    vertexShader = orbisGlCompileShader(GL_VERTEX_SHADER, s_vertex_shader_code);
    if (!vertexShader) {
        debugNetPrintf(DEBUG, "Error during compiling vertex shader !\n");
    }

    fragmentShader = orbisGlCompileShader(GL_FRAGMENT_SHADER, s_fragment_shader_code);
    if (!fragmentShader) {
        debugNetPrintf(DEBUG, "Error during compiling fragment shader !\n");
    }

    programID = orbisGlLinkProgram(vertexShader, fragmentShader);
    if (!programID) {
        debugNetPrintf(DEBUG, "Error during linking shader ! program_id=%d (0x%08x)\n", programID, programID);
    }

#else // links MiniAPI !
    programID = miniCompileShaders(s_vertex_shader_code, s_fragment_shader_code);

#endif
    // feedback
    debugNetPrintf(DEBUG, "program_id=%d (0x%08x)\n", programID, programID);
    return programID;
}

// libfreetype-gl pass last composed Text_Length in pixel, we use to align text
extern float tl;
// ------------------------------------------------------------------- main ---
int es2sample_init( int argc, char **argv )
{
    debugNetPrintf(DEBUG,"es2sample_init\n");

    size_t h; // text size in pt
    texture_font_t  *font  = 0;
    atlas = texture_atlas_new( 1024, 1024, 1 );  // a bigger atlas, stores more glyphs
    debugNetPrintf(DEBUG,"atlas at %p\n", atlas);

    const char * filename  = "host0:SoMAItalicBold.ttf";

    wchar_t *text = L"OrbisLinkGL demo: GL ES 2.0 shaders, FreeType";

    buffer = vertex_buffer_new( "vertex:3f,tex_coord:2f,color:4f" );
    debugNetPrintf(DEBUG,"buffer at %p\n", buffer);

    int tx = 20; // text position

    vec2 pen = {{500, 600}}; // init pen: 0,0 is lower left

    vec4 black = {{ 0.1, 0.1, 0.4, 1 }}; // RGBA color
    vec4 white = {{ 1,   1,   1,   1 }}; // RGBA color
    vec4 col   = {{ 1,   0.2, 1,   1 }}; // RGBA color

#ifdef __PS4__
    font = texture_font_new( atlas, filename, 32 );  // create a font

    // 1.
    wchar_t *s = L"this sample show aligned text";   // set text

    texture_font_load_glyphs( font, s );        // set textures
    pen.x = (ATTR_ORBISGL_WIDTH - tl) /2;       // use Text_Length to align pen.x
    add_text( buffer, font, s, &white, &pen );  // set vertexes

    // 2.
    s = L"liborbis freetype demo";  // set text
    texture_font_load_glyphs( font, s );        // set textures
    pen.x = (ATTR_ORBISGL_WIDTH - tl);          // use Text_Length to align pen.x (right)
    pen.y -= font->height;                      // 1 line down!
    add_text( buffer, font, s, &white, &pen );  // set vertexes

    texture_font_delete( font );    // end with font, cleanup

    if(1)
    {
        font = texture_font_new( atlas, "host0:razors.ttf", 36 );  // create a font
        wchar_t *s = L"make#liborbis";            // set text
        texture_font_load_glyphs( font, s );      // set textures
        pen.x = (ATTR_ORBISGL_WIDTH - tl) /2;     // set x position
        pen.y = 900;                              // set y position
        add_text( buffer, font, s, &col, &pen );  // set vertexes
        texture_font_delete( font );              // cleanup font
    }
#endif
    pen.y = 800;
    for( h=12; h < 72; h += 6)
    {
        font = texture_font_new( atlas, "host0:SoMALightItalic.ttf", h );
        //debugNetPrintf(DEBUG,"font at %p\n", font);
        // reset pen, 1 line down
        pen.x = tx;
        pen.y -= font->height;

        texture_font_load_glyphs( font, text );

        // use Text_Length to align pen.x // printf("main: tl %f\n", tl);
        pen.x = (ATTR_ORBISGL_WIDTH - tl) /2;
        add_text( buffer, font, text, &white, &pen );

        texture_font_delete( font );
    }
    glBindTexture( GL_TEXTURE_2D, atlas->id );

    // compile, link and use shader
    shader = CreateProgram();

    if(!shader)
    {
        debugNetPrintf(DEBUG,"program creation failed\n");
    }

    mat4_set_identity( &projection );
    mat4_set_identity( &model );
    mat4_set_identity( &view );


    reshape(ATTR_ORBISGL_WIDTH, ATTR_ORBISGL_HEIGHT);

    return 0;
}

