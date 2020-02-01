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

#include <string.h>

#include <freetype-gl.h>  // links against libfreetype-gl


#if defined(__APPLE__)
    #include <Glut/glut.h>
#elif defined(_WIN32) || defined(_WIN64)
    #include <GLUT/glut.h>
#elif defined(__PS4__)
    #include <debugnet.h>
    #include <orbisFile.h>

    extern size_t _orbisFile_lastopenFile_size;
#else
    #include <GL/glut.h>
#endif

#ifdef _MAPI_
#include <MiniShader.h>  // miniCompileShaders ()
#endif

// ------------------------------------------------------- typedef & struct ---
typedef struct {
    float x, y, z;    // position
    float s, t;       // texture
    float r, g, b, a; // color
} vertex_t;



// ------------------------------------------------------- global variables ---
GLuint shader;

static texture_atlas_t *atlas  = NULL;
static vertex_buffer_t *buffer = NULL;
static mat4             model, view, projection;

// ---------------------------------------------------------------- reshape ---
static void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    mat4_set_orthographic( &projection, 0, width, 0, height, -1, 1);
}

// ---------------------------------------------------------------- display ---
void render_text( void )
{
    // we already clean in main renderloop()!

    glUseProgram( shader );
    // setup state
    glBindTexture( GL_TEXTURE_2D, atlas->id ); // rebind glyph atlas
    glDisable(GL_CULL_FACE); // just in case, disable it

    // freetype-gl shaders wanted config
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    //glUseProgram( shader );
    {
        glUniform1i       ( glGetUniformLocation( shader, "texture" ),    0 );
        glUniformMatrix4fv( glGetUniformLocation( shader, "model" ),      1, 0, model.data);
        glUniformMatrix4fv( glGetUniformLocation( shader, "view" ),       1, 0, view.data);
        glUniformMatrix4fv( glGetUniformLocation( shader, "projection" ), 1, 0, projection.data);

        vertex_buffer_render( buffer, GL_TRIANGLES );
    }

    glDisable( GL_BLEND ); // Reset state back

    // we already swapframe in main renderloop()!
}


// --------------------------------------------------------------- add_text ---
static void add_text( vertex_buffer_t * buffer, texture_font_t * font,
               char * text, vec4 * color, vec2 * pen )
{
    size_t i;
    float r = color->red, g = color->green, b = color->blue, a = color->alpha;


    for( i = 0; i < strlen(text); ++i )
    {
        texture_glyph_t *glyph = texture_font_get_glyph( font, text + i );
        if( glyph != NULL )
        {
            float kerning = 0.0f;
            if(i > 0)
            {
                kerning = texture_glyph_get_kerning( glyph, text + i - 1 );
            }
            pen->x  += kerning;
            int   x0 = (int)( pen->x + glyph->offset_x );
            int   y0 = (int)( pen->y + glyph->offset_y );
            int   x1 = (int)( x0 + glyph->width );
            int   y1 = (int)( y0 - glyph->height );

            float s0 = glyph->s0;
            float t0 = glyph->t0;
            float s1 = glyph->s1;
            float t1 = glyph->t1;

            GLuint indices[6] = {0,1,2, 0,2,3}; // (two triangles)
            /* VBO is setup as: "vertex:3f, tex_coord:2f, color:4f" */
            vertex_t vertices[4] = { { x0,y0,0,   s0,t0,   r,g,b,a },
                                     { x0,y1,0,   s0,t1,   r,g,b,a },
                                     { x1,y1,0,   s1,t1,   r,g,b,a },
                                     { x1,y0,0,   s1,t0,   r,g,b,a } };
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


    /* we can use OrbisGl wrappers, or MiniAPI ones */
#ifndef _MAPI_

    GLuint vertexShader;
    GLuint fragmentShader;

    vertexShader = orbisGlCompileShader(GL_VERTEX_SHADER, s_vertex_shader_code);

    if(!vertexShader)
        { debugNetPrintf(DEBUG, "Error during compiling vertex shader !\n"); }

    fragmentShader = orbisGlCompileShader(GL_FRAGMENT_SHADER, s_fragment_shader_code);
    if(!fragmentShader)
        { debugNetPrintf(DEBUG, "Error during compiling fragment shader !\n"); }

    programID = orbisGlLinkProgram(vertexShader, fragmentShader);
    if(!programID)
        { debugNetPrintf(DEBUG, "Error during linking shader ! program_id=%d (0x%08x)\n", programID, programID); }

#else /* include and links against MiniAPI library! */
    programID = miniCompileShaders(s_vertex_shader_code, s_fragment_shader_code);

#endif
    //debugNetPrintf(DEBUG, "program_id=%d (0x%08x)\n", programID, programID);
    return programID;
}


// libfreetype-gl pass last composed Text_Length in pixel, we use to align text
extern float tl;

/* for texts */
static int  tx    = 20;           // text position
       vec2 pen   = {{500, 600}}; // init pen: 0,0 is lower left
static vec4 white = {{ 1,   1,   1,   1 }};      // RGBA color
       vec4 col   = {{ 1,   0.2, 1,   1 }};      // RGBA color
    // vec4 black = {{ 0.1, 0.1, 0.4, 1 }};      // RGBA color
static texture_font_t *font = NULL;
// ------------------------------------------------------------------- main ---
int es2sample_init(void)
{
    //debugNetPrintf(DEBUG,"es2sample_init\n");
    size_t h; // text size in pt

    /* init page: compose all texts to draw */
    atlas  = texture_atlas_new( 512, 512, 1 );

    /* load .ttf in memory */
    void *ttf = orbisFileGetFileContent( "host0:SoMAItalicBold.ttf" );
    buffer    = vertex_buffer_new( "vertex:3f,tex_coord:2f,color:4f" );
    //debugNetPrintf(DEBUG,"buffer at %p, atlas at %p\n", buffer, atlas);

    char *text = "OrbisLinkGL demo: GL ES 2.0 shaders, FreeType";

#ifdef __PS4__
  //font = texture_font_new( atlas, "host0:SoMAItalicBold.ttf", 32 );  // create a font
    font = texture_font_new_from_memory(atlas, 32, ttf, _orbisFile_lastopenFile_size);
    // 1.
    char *s = "this sample show aligned text";  // set text

    texture_font_load_glyphs( font, s );        // set textures
    pen.x = (ATTR_ORBISGL_WIDTH - tl) /2;       // use Text_Length to align pen.x
    pen.y = ATTR_ORBISGL_HEIGHT /2;             // centered over Y
    add_text( buffer, font, s, &white, &pen );  // set vertexes

    // 2.
    s = "liborbis freetype demo";               // set text

    texture_font_load_glyphs( font, s );        // set textures
    pen.x = (ATTR_ORBISGL_WIDTH - tl);          // use Text_Length to align pen.x (right)
    pen.y -= font->height;                      // 1 line down!
    add_text( buffer, font, s, &white, &pen );  // set vertexes


    texture_font_delete( font ); // done with this font, cleanup

    if(1)
    {
      //font = texture_font_new( atlas, "host0:razors.ttf", 36 ); // create a font
        ttf  = orbisFileGetFileContent("host0:razors.ttf");
        font = texture_font_new_from_memory(atlas, 36, ttf, _orbisFile_lastopenFile_size);

        char *s = "make#liborbis";                // set text
        texture_font_load_glyphs( font, s );      // set textures
        pen.x = (ATTR_ORBISGL_WIDTH - tl) /2;     // set x position
        pen.y = 900;                              // set y position
        font->rendermode = RENDER_OUTLINE_EDGE;   // set outline
        font->outline_thickness = 1;
        add_text( buffer, font, s, &col, &pen );  // set vertexes

        texture_font_delete( font );              // cleanup font
    }
#endif

    ttf   = orbisFileGetFileContent("host0:SoMALightItalic.ttf");
    pen.y = 800;
    for( h=12; h < 72; h += 6)
    {
      //font = texture_font_new( atlas, "host0:SoMALightItalic.ttf", h ); // create a font
        font = texture_font_new_from_memory(atlas, h, ttf, _orbisFile_lastopenFile_size);

        //debugNetPrintf(DEBUG,"font at %p\n", font);
        pen.x  = tx;
        pen.y -= font->height;  // reset pen, 1 line down

        texture_font_load_glyphs( font, text );

        pen.x = (ATTR_ORBISGL_WIDTH - tl) /2; // use Text_Length to align pen.x


        add_text( buffer, font, text, &white, &pen );

        texture_font_delete( font );
    }


    /* create texture and upload atlas into */
    glGenTextures  ( 1, &atlas->id );
    glBindTexture  ( GL_TEXTURE_2D, atlas->id );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D   ( GL_TEXTURE_2D, 0, GL_ALPHA, atlas->width, atlas->height,
                                    0, GL_ALPHA, GL_UNSIGNED_BYTE, atlas->data );

    /* compile, link and use shader */
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


void es2sample_end(void)
{
    texture_atlas_delete(atlas),  atlas  = NULL;
    vertex_buffer_delete(buffer), buffer = NULL;

    if(shader)
        glDeleteProgram(shader), shader = 0;

    return;
}
