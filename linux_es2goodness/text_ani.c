#include <stdio.h>
#include <string.h>

/*clang *.c -I$PS4SDK/include/MiniAPI -L/Archive/PS4-work/OrbisLink/samples/pc_es2template/source -lMiniAPI -L/Archive/PS4-work/OrbisLink/samples/pc_es2template/source -lm -lGL -lEGL -lX11 -D_MAPI_ -I$PS4SDK/include/freetype2 -lfreetype -png -ggdb*/

#if defined(__APPLE__)
    #include <Glut/glut.h>
#elif defined(_WIN32) || defined(_WIN64)
    #include <GLUT/glut.h>
#elif defined(__PS4__)
    #include <debugnet.h>
    #include <orbisFile.h>
#endif


#include "defines.h"


// ------------------------------------------------------- typedef & struct ---
typedef struct {
    float x, y, z;    // position (3f)
    float s, t;       // texture  (2f)
    float r, g, b, a; // color    (4f)
} vertex_t;


//#include "MiniShader.h"  // miniCompileShaders


/* basic indexing of lines/texts */
struct item_entry
{
    // to address item in vertex_buffer_t:
    int off,     // from VBO head, in glyphs
        len;     // = strlen(text)
};

static int    itemcount = 0;
static struct item_entry item[4]; // max 4 structs

// ------------------------------------------------------- global variables ---
// shader and locations
static GLuint program    = 0; // default program
static GLuint shader_fx  = 0; // text_ani.[vf]
static mat4   model, view, projection;
static float  g_Time     = 0.0f;
static GLuint g_TimeSlot = 0;
// ---------------------------------------------------------------- reshape ---
static void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    mat4_set_orthographic( &projection, 0, width, 0, height, -1, 1);
}

// ----------------------------------------------------------text animation ---



static int         selected = 0; // we draw one text at once
static fx_entry_t *ani;          // the fx info



// ---------------------------------------------------------------- display ---
static void render_text_extended( int text_num, int type_num )
{
    // we already clean in main renderloop()!

    //type_num = 1; // which fx_entry_t test

    fx_entry_t *ani = &fx_entry[type_num],
               *fx  = &fx_entry[0];
    int t_n = ani - fx;

    program         = shader_fx;
#if 1
    if(ani->fcount >= ani->life) // looping ani_state, foreach text
    {
        switch(ani->status) // setup for next fx
        {
            case CLOSED : ani->status = IN;      ani->life  =  30.; break;
            case IN     : ani->status = DEFAULT; ani->life  = 240.; break;
            case DEFAULT: ani->status = OUT;     ani->life  =  30.; break;
            case OUT    : ani->status = CLOSED;  ani->life  =  60.,
            /* CLOSED reached: switch text! */   selected  +=   1 ; break;
        }
        ani->fcount = 0; // reset framecount
    }
#endif
/*
    printf("program: %d [%d] fx state: %.1f, frame: %3d/%3.f %.3f\r", 
            program, t_n,
                     ani->status /10.,
                     ani->fcount,
                     ani->life,
                     fmod(ani->status /10. + type_num /100., .02));
*/
    glUseProgram   ( program );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture  ( GL_TEXTURE_2D, atlas->id ); // rebind glyph atlas
    glDisable      ( GL_CULL_FACE );
    glEnable       ( GL_BLEND );
    glBlendFunc    ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    {
        glUniform1i       ( glGetUniformLocation( program, "texture" ),    0 );
        glUniformMatrix4fv( glGetUniformLocation( program, "model" ),      1, 0, model.data);
        glUniformMatrix4fv( glGetUniformLocation( program, "view" ),       1, 0, view.data);
        glUniformMatrix4fv( glGetUniformLocation( program, "projection" ), 1, 0, projection.data);
        glUniform4f       ( glGetUniformLocation( program, "meta"), 
                ani->fcount,
                ani->status /10., // we use float on SL, swtching fx state
                ani->life,
                type_num    /10.);

        if(0) /* draw whole VBO (storing all added texts) */
        {
           vertex_buffer_render( buffer, GL_TRIANGLES );
        }
        else /* draw a range/selection of indeces (= glyph) */
        {
            vertex_buffer_render_setup( buffer, GL_TRIANGLES ); // start draw
            
            //for(int j=0; j<itemcount; j+=1)  // draw all texts
            //int j = selected %NUM;
            int j = text_num;
            {
                // draw just text[j]
                // iterate each char in text[j]
                for(int i  = item[j].off;
                        i  < item[j].off + item[j].len;
                        i += 1)
                { // glyph by one (2 triangles: 6 indices, 4 vertices)
                    vertex_buffer_render_item ( buffer, i );
                }
            }
            
            vertex_buffer_render_finish( buffer ); // end draw
        }
    }
    glDisable( GL_BLEND ); 

    ani->fcount    += 1;

    // we already swapframe in main renderloop()!
}


/* wrapper from main */
void render_text_ext( fx_entry_t *_ani )
{
    //for (int i = 0; i < itemcount; ++i)
    {
    /*  render_text_extended( item_entry, fx_entry );
        read as: draw this text using this effect! */
        render_text_extended( 0, TYPE_0 );
        render_text_extended( 1, TYPE_1 );
        render_text_extended( 2, TYPE_2 );
        render_text_extended( 3, TYPE_3 );
    }
}


/* shared freetype-gl function! */
void add_text( vertex_buffer_t * buffer, texture_font_t * font,
               char * text, vec4 * color, vec2 * pen );
/* 
 my wrapper for standard ft-gl2 add_text(), but with
 added texts indexing to render then by selection;

 we append to default ft-gl2 VBOs, atlas and texture!
*/
static void my_add_text( vertex_buffer_t * buffer, texture_font_t * font,
                         char * text, vec4 * color, vec2 * pen,
                         /* to store out text index */
                         struct item_entry * idx )
{   /* index text */
    idx->off = vector_size( buffer->items );
    /* set vertexes in VBO and glyph UVs by texture */
    add_text( buffer, font, text, color, pen );
    /* save glyph count */
    idx->len = vector_size( buffer->items ) - idx->off;
    int curr = idx - item;
    /* report the item info */
    printf("item[%.2d] .off: %3d, .len: %2d, buffer glyph count: %3zu, %s\n",
            curr, idx->off, idx->len, vector_size( buffer->items ), text );
    /* keep track on itemcount */
    itemcount += 1;
}

// --------------------------------------------------------- custom shaders ---
static GLuint CreateProgram( void )
{
#if 1 /* we can use OrbisGl wrappers, or MiniAPI ones */
    const GLchar  *vShader = (void*) orbisFileGetFileContent( "shaders/text_ani.vert" );
    const GLchar  *fShader = (void*) orbisFileGetFileContent( "shaders/text_ani.frag" );
          GLuint programID = BuildProgram(vShader, fShader); // shader_common.c

#endif /* include and links against MiniAPI library! */
    //programID = miniCompileShaders(s_vertex_shader_code, s_fragment_shader_code);

    if (!programID) { sleep(2); }
    // feedback
    printf( "program_id=%d (0x%08x)\n", programID, programID);
    return programID;
}


// libfreetype-gl pass last composed Text_Length in pixel, we use to align text!
extern float tl;
static texture_font_t *font = NULL;
// ------------------------------------------------------------------- main ---
void es2init_text_ani(int width, int height)
{
	// build item list, nfs-ls.c	
	//process_dir2(nfs, "", 16);
    
	// we don't know now itemcount yet

    /* init page: compose all texts to draw */

    /* load .ttf in memory */
    void *ttf  = orbisFileGetFileContent( "fonts/zrnic_rg.ttf" );
    //printf("buffer at %p, atlas at %p, atlas->id:%d\n", buffer, atlas, atlas->id);

    /* reuse texture_atlas! */
    //if(!atlas) atlas  = texture_atlas_new( 512, 512, 1 );
         font  = texture_font_new_from_memory(atlas, 32, ttf, _orbisFile_lastopenFile_size);
    vec2 pen   = {{ 200, 480 /2 }}; // init pen: 0,0 is lower left
    vec4 white = {{ 1.f, 1.f, 1.f, 1.f }}; // RGBA color
    vec4 col   = {{ 1.f, 0.f, 0.4, 1.f }}; // RGBA color
    /* reuse main demo-font VBO! */
	//buffer   = vertex_buffer_new( "vertex:3f,tex_coord:2f,color:4f" );

for (int i = 0; i < NUM; ++i)
{
    // now append all text to VBO and atlas

    // setup positions for each text we render later
	texture_font_load_glyphs( font, text[i] );

	// after texture_font_load_glyphs() calls, 'tl' holds text len in pixels!
	//pen.x  = (1024 - tl) /2; // use Text_Length to align pen.x 
    //pen.y -= 16;
    if(i == TYPE_3) pen = (vec2){ 10, 64 };

    my_add_text( buffer, font, text[i], &white, &pen, &item[i] );
}

    texture_font_delete( font );

    /* discard old texture, we eventually added glyphs! */
    if(atlas->id) glDeleteTextures(1, &atlas->id), atlas->id = 0;

    /* re-create texture and upload atlas into gpu memory */
    glGenTextures  ( 1, &atlas->id );
    glBindTexture  ( GL_TEXTURE_2D, atlas->id );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D   ( GL_TEXTURE_2D, 0, GL_ALPHA, atlas->width, atlas->height,
                                    0, GL_ALPHA, GL_UNSIGNED_BYTE, atlas->data );
	/* shader program is custom, so
       compile, link and use shader */
    shader_fx = CreateProgram();

    if(!shader_fx) { printf("program creation failed\n"); }

    /* init ani effect */
    ani = &fx_entry[0];
    ani->status = CLOSED,
    ani->fcount =  0;     // framecount
    ani->life   = 20.f;   // duration in frames

    mat4_set_identity( &projection );
    mat4_set_identity( &model );
    mat4_set_identity( &view );
    // attach our "custom infos"
    g_TimeSlot = glGetUniformLocation(shader_fx, "meta");

    reshape(width, height);
}

void es2update_text_ani(double elapsedTime)
{
    g_Time += (float)(elapsedTime * 1.f);  // adjust time
    //glUseProgram(shader_fx);
//printf("%d:%.4f %d %4G %4f\n", g_TimeSlot, g_Time, ani_status, elapsedTime, elapsedTime);
    //glUniform1f(g_TimeSlot, g_Time); // notify shader about elapsed time
}

void es2fini_text_ani( void )
{
    //texture_atlas_delete(atlas),  atlas  = NULL;
    //vertex_buffer_delete(buffer), buffer = NULL;
    if(shader_fx) glDeleteProgram(shader_fx), shader_fx = 0;
}
