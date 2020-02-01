/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#include <stdio.h>
#include <string.h>

#include <freetype-gl.h>  // links against libfreetype-gl

#if defined(__PS4__)
    #include <debugnet.h>
    #include <orbisFile.h>
// each orbisFileGetFileContent() call will update filesize!
extern size_t _orbisFile_lastopenFile_size;

#else
    #include <GLFW/glfw3.h>
#endif


// ------------------------------------------------------- typedef & struct ---
typedef struct {
    float x, y, z;    // position
    float s, t;       // texture
    float r, g, b, a; // color
} vertex_t;


// ------------------------------------------------------- global variables ---
GLuint shader;
vertex_buffer_t *buffer;
texture_atlas_t *atlas;
mat4   model, view, projection;


// ---------------------------------------------------------------- display ---
void render( void )
{
    glClearColor( 0.3, 0.1, 0.9, 1 ); // BGRA
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glUseProgram( shader );
    {
        glUniform1i       ( glGetUniformLocation( shader, "texture" ),    0 );
        glUniformMatrix4fv( glGetUniformLocation( shader, "model" ),      1, 0, model.data);
        glUniformMatrix4fv( glGetUniformLocation( shader, "view" ),       1, 0, view.data);
        glUniformMatrix4fv( glGetUniformLocation( shader, "projection" ), 1, 0, projection.data);
        /* draw whole VBO item array */
        vertex_buffer_render( buffer, GL_TRIANGLES );
    }
    #ifndef __PS4__
    glfwSwapBuffers( window );
    #endif
}

// ---------------------------------------------------------------- reshape ---
void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    mat4_set_orthographic( &projection, 0, width, 0, height, -1, 1);
}

// --------------------------------------------------------------- add_text ---
void add_text( vertex_buffer_t * buffer, texture_font_t * font,
               char * text, vec4 * color, vec2 * pen )
{
    size_t i;
    float r = color->red, g = color->green, b = color->blue, a = color->alpha;
    for( i = 0; i < strlen(text); ++i )
    {
        texture_glyph_t *glyph = texture_font_get_glyph( font, text + i );
        if( glyph != NULL )
        {
            float kerning =  0.0f;
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
            GLuint indices[6] = {0,1,2, 0,2,3};
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
static GLuint CreateProgram(void)
{
    GLuint programID = 0;

    /* we can use OrbisGl wrappers, or MiniAPI ones */
#ifndef _MAPI_
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

#else /* include and links against MiniAPI library! */
    programID = miniCompileShaders(s_vertex_shader_code, s_fragment_shader_code);

#endif
    debugNetPrintf(DEBUG, "program_id=%d (0x%08x)\n", programID, programID);
    return programID;
}

// libfreetype-gl pass last composed Text_Length in pixel, we use to align text
extern float tl;

// ------------------------------------------------------------------- init ---
void es2sample_init( void )
{
    size_t h; // text size in pt
    texture_font_t *font = 0;
    atlas = texture_atlas_new( 512, 512, 1 );

    /* load .ttf in memory */
    void *ttf  = orbisFileGetFileContent("host0:zrnic_rg.ttf");

    char *text = "A Quick Brown Fox Jumps Over The Lazy Dog 0123456789 @#àèìòù";

    buffer     = vertex_buffer_new( "vertex:3f,tex_coord:2f,color:4f" );
    vec2 pen   = {{ 5, ATTR_ORBISGL_HEIGHT /2 +100}}; // init pen: 0,0 is lower left
    vec4 white = {{ 1,   1,   1,   1 }};              // pick an RGBA color
    for(h=24; h < 32; ++h)
    {
        //font = texture_font_new_from_file( atlas, h, filename );
        font = texture_font_new_from_memory(atlas, h, ttf, _orbisFile_lastopenFile_size);
        pen.y -= font->height; // 1 line down

        texture_font_load_glyphs( font, text );

        pen.x = (ATTR_ORBISGL_WIDTH - tl) /2; // use Text_Length to align pen.x

        debugNetPrintf(DEBUG,"size %.2zupt, text length: %.2fpx\n", h, tl);

        add_text( buffer, font, text, &white, &pen );
        texture_font_delete( font );
    }
    /* done with .ttf, release buffer */
    if(ttf) free(ttf), ttf = NULL;

    /* create texture and upload atlas into */
    glGenTextures( 1, &atlas->id );
    glBindTexture( GL_TEXTURE_2D, atlas->id );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, atlas->width, atlas->height,
                                 0, GL_ALPHA, GL_UNSIGNED_BYTE, atlas->data );

    /* compile, link and use shader */
    #ifndef __PS4__ // disable on PS4
    shader = shader_load("shaders/v3f-t2f-c4f.vert",
                         "shaders/v3f-t2f-c4f.frag");
    #else
    /* load from HOST0
    shader = orbisGlCreateProgram("host0:shaders/v3f-t2f-c4f.vert",
                                  "host0:shaders/v3f-t2f-c4f.frag"); */
    /* use stock freetype-gl shader */
    shader = CreateProgram();

    #endif
    if(!shader)
    {
        debugNetPrintf(DEBUG,"program creation failed\n");
        sleep(1);
    }

    mat4_set_identity( &projection );
    mat4_set_identity( &model );
    mat4_set_identity( &view );

    #ifdef __PS4__
    reshape(ATTR_ORBISGL_WIDTH, ATTR_ORBISGL_HEIGHT);
    #endif
}

void es2sample_end( void )
{
    texture_atlas_delete(atlas);  atlas  = NULL;
    vertex_buffer_delete(buffer); buffer = NULL;

    if(shader)
        glDeleteProgram(shader);

    shader = 0;
    return;
}


#ifndef __PS4__ // disable on PS4
// --------------------------------------------------------------- keyboard ---
void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
    if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
    {
        glfwSetWindowShouldClose( window, GL_TRUE );
    }
}


// --------------------------------------------------------- error-callback ---
void error_callback( int error, const char* description )
{
    fputs( description, stderr );
}


// ------------------------------------------------------------------- main ---
int main( int argc, char **argv )
{
    GLFWwindow* window;
    char* screenshot_path = NULL;

    if (argc > 1)
    {
        if (argc == 3 && 0 == strcmp( "--screenshot", argv[1] ))
            screenshot_path = argv[2];
        else
        {
            fprintf( stderr, "Unknown or incomplete parameters given\n" );
            exit( EXIT_FAILURE );
        }
    }

    glfwSetErrorCallback( error_callback );

    if (!glfwInit( ))
    {
        exit( EXIT_FAILURE );
    }

    glfwWindowHint( GLFW_VISIBLE, GL_FALSE );
    glfwWindowHint( GLFW_RESIZABLE, GL_FALSE );

    window = glfwCreateWindow( 800, 500, argv[0], NULL, NULL );

    if (!window)
    {
        glfwTerminate( );
        exit( EXIT_FAILURE );
    }

    glfwMakeContextCurrent( window );
    glfwSwapInterval( 1 );

    glfwSetFramebufferSizeCallback( window, reshape );
    glfwSetWindowRefreshCallback( window, display );
    glfwSetKeyCallback( window, keyboard );

#ifndef __APPLE__
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf( stderr, "Error: %s\n", glewGetErrorString(err) );
        exit( EXIT_FAILURE );
    }
    fprintf( stderr, "Using GLEW %s\n", glewGetString(GLEW_VERSION) );
#endif

    init();

    glfwShowWindow( window );
    reshape( window, 800, 500 );

    while (!glfwWindowShouldClose( window ))
    {
        display( window );
        glfwPollEvents( );

        if (screenshot_path)
        {
            screenshot( window, screenshot_path );
            glfwSetWindowShouldClose( window, 1 );
        }
    }

    glDeleteTextures( 1, &atlas->id );
    atlas->id = 0;
    texture_atlas_delete( atlas );

    glfwDestroyWindow( window );
    glfwTerminate( );

    return EXIT_SUCCESS;
}
#endif  // __PS4__ disable on PS4