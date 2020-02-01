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

typedef struct {
    float x, y, z;
    vec4 color;
} point_t;


// ------------------------------------------------------- global variables ---
texture_atlas_t *atlas;
vertex_buffer_t *text_buffer;
vertex_buffer_t *line_buffer;
vertex_buffer_t *point_buffer;
GLuint shader, text_shader;
mat4 model, view, projection;
const int width  = ATTR_ORBISGL_WIDTH;
const int height = ATTR_ORBISGL_HEIGHT;

// ---------------------------------------------------------------- display ---
void render( void )
{
    /*int viewport[4];
    glGetIntegerv( GL_VIEWPORT, viewport );*/
    /*glClearColor(1,1,1,1);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );*/
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    
    glUseProgram( text_shader );
    {
        glUniform1i       ( glGetUniformLocation( text_shader, "texture" ),    0 );
        glUniformMatrix4fv( glGetUniformLocation( text_shader, "model" ),      1, 0, model.data);
        glUniformMatrix4fv( glGetUniformLocation( text_shader, "view" ),       1, 0, view.data);
        glUniformMatrix4fv( glGetUniformLocation( text_shader, "projection" ), 1, 0, projection.data);
        /* draw whole VBO item array */
        vertex_buffer_render( text_buffer, GL_TRIANGLES );
    }

    //glPointSize( 10.0f );

    glUseProgram( shader );
    {
        glUniformMatrix4fv( glGetUniformLocation( shader, "model" ),      1, 0, model.data);
        glUniformMatrix4fv( glGetUniformLocation( shader, "view" ),       1, 0, view.data);
        glUniformMatrix4fv( glGetUniformLocation( shader, "projection" ), 1, 0, projection.data);
        /* draw whole VBO item arrays */
        vertex_buffer_render( line_buffer,  GL_LINES );
        vertex_buffer_render( point_buffer, GL_POINTS );
    }
    glUseProgram( 0 );

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
               char *text, vec4 * color, vec2 * pen )
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
            GLuint indices[] = {0,1,2, 0,2,3};
            /* VBO is setup as: "vertex:3f, tex_coord:2f, color:4f" */
            vertex_t vertices[] = { { x0,y0,0,   s0,t0,   r,g,b,a },
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
    vec4 blue  = {{0,0,1,1}};
    vec4 black = {{0,0,0,1}};

    /* load .ttf in memory */
    void *ttf  = orbisFileGetFileContent("host0:zrnic_rg.ttf");

    atlas = texture_atlas_new( 512, 512, 1 );
    texture_font_t *big   = texture_font_new_from_memory(atlas, 400, ttf, _orbisFile_lastopenFile_size);
    texture_font_t *small = texture_font_new_from_memory(atlas,  18, ttf, _orbisFile_lastopenFile_size);
    texture_font_t *title = texture_font_new_from_memory(atlas,  32, ttf, _orbisFile_lastopenFile_size);

    text_buffer  = vertex_buffer_new( "vertex:3f,tex_coord:2f,color:4f" );
    line_buffer  = vertex_buffer_new( "vertex:3f,color:4f" );
    point_buffer = vertex_buffer_new( "vertex:3f,color:4f" );

    vec2 pen, origin;

    texture_glyph_t *glyph  = texture_font_get_glyph( big, "g" );
    origin.x = width/2  - glyph->offset_x - glyph->width/2;
    origin.y = height/2 - glyph->offset_y + glyph->height/2;
    add_text( text_buffer, big, "g", &black, &origin );

    // title
    pen.x = 50;
    pen.y = ATTR_ORBISGL_HEIGHT - 50;
    add_text( text_buffer, title, "Glyph metrics", &black, &pen );

    point_t vertices[] =
        {   // Baseline
            {0.1*width, origin.y, 0, black},
            {0.9*width, origin.y, 0, black},

            // Top line
            {0.1*width, origin.y + glyph->offset_y, 0, black},
            {0.9*width, origin.y + glyph->offset_y, 0, black},

            // Bottom line
            {0.1*width, origin.y + glyph->offset_y - glyph->height, 0, black},
            {0.9*width, origin.y + glyph->offset_y - glyph->height, 0, black},

            // Left line at origin
            {width/2-glyph->offset_x-glyph->width/2, 0.1*height, 0, black},
            {width/2-glyph->offset_x-glyph->width/2, 0.9*height, 0, black},

            // Left line
            {width/2 - glyph->width/2, .3*height, 0, black},
            {width/2 - glyph->width/2, .9*height, 0, black},

            // Right line
            {width/2 + glyph->width/2, .3*height, 0, black},
            {width/2 + glyph->width/2, .9*height, 0, black},

            // Right line at origin
            {width/2-glyph->offset_x-glyph->width/2+glyph->advance_x, 0.1*height, 0, black},
            {width/2-glyph->offset_x-glyph->width/2+glyph->advance_x, 0.7*height, 0, black},

            // Width
            {width/2 - glyph->width/2, 0.8*height, 0, blue},
            {width/2 + glyph->width/2, 0.8*height, 0, blue},

            // Advance_x
            {width/2-glyph->width/2-glyph->offset_x, 0.2*height, 0, blue},
            {width/2-glyph->width/2-glyph->offset_x+glyph->advance_x, 0.2*height, 0, blue},

            // Offset_x
            {width/2-glyph->width/2-glyph->offset_x, 0.85*height, 0, blue},
            {width/2-glyph->width/2, 0.85*height, 0, blue},

            // Height
            {0.3*width/2, origin.y + glyph->offset_y - glyph->height, 0, blue},
            {0.3*width/2, origin.y + glyph->offset_y, 0, blue},

            // Offset y
            {0.8*width, origin.y + glyph->offset_y, 0, blue},
            {0.8*width, origin.y , 0, blue},

        };
    GLuint indices [] = {  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,
                          13,14,15,16,17,18,19,20,21,22,23,24,25};
    vertex_buffer_push_back( line_buffer, vertices, 26, indices, 26 );

    pen.x = width/2 - 48;
    pen.y = .2*height - 18;
    add_text( text_buffer, small, "advance_x", &blue, &pen );

    pen.x = width/2 - 20;
    pen.y = .8*height + 3;
    add_text( text_buffer, small, "width", &blue, &pen );

    pen.x = width/2 - glyph->width/2 + 5;
    pen.y = .85*height-8;
    add_text( text_buffer, small, "offset_x", &blue, &pen );

    pen.x = 0.2*width/2-30;
    pen.y = origin.y + glyph->offset_y - glyph->height/2;
    add_text( text_buffer, small, "height", &blue, &pen );

    pen.x = 0.8*width+3;
    pen.y = origin.y + glyph->offset_y/2 -6;
    add_text( text_buffer, small, "offset_y", &blue, &pen );

    pen.x = width/2  - glyph->offset_x - glyph->width/2 - 58;
    pen.y = height/2 - glyph->offset_y + glyph->height/2 - 20;
    add_text( text_buffer, small, "Origin", &black, &pen );

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

    GLuint i = 0;
    point_t p;
    p.color = black;

    // Origin point
    p.x = width/2  - glyph->offset_x - glyph->width/2;
    p.y = height/2 - glyph->offset_y + glyph->height/2;
    vertex_buffer_push_back( point_buffer, &p, 1, &i, 1 );

    // Advance point
    p.x = width/2  - glyph->offset_x - glyph->width/2 + glyph->advance_x;
    p.y = height/2 - glyph->offset_y + glyph->height/2;
    vertex_buffer_push_back( point_buffer, &p, 1, &i, 1 );

    /* compile, link and use shader */
    #ifndef __PS4__ // disable on PS4
    text_shader = shader_load( "shaders/v3f-t2f-c4f.vert",
                               "shaders/v3f-t2f-c4f.frag" );

    shader = shader_load( "shaders/v3f-c4f.vert",
                          "shaders/v3f-c4f.frag" );
    #else
    /* use stock freetype-gl shader */
    text_shader = CreateProgram();
    
    /* load from HOST0 */
    shader = orbisGlCreateProgram("host0:shaders/v3f-c4f.vert",
                                  "host0:shaders/v3f-c4f.frag");
    #endif
    if(!text_shader
    || !shader)
    {
        debugNetPrintf(DEBUG,"program creation failed\n");
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
    texture_atlas_delete(atlas);        atlas        = NULL;
    vertex_buffer_delete(text_buffer);  text_buffer  = NULL;
    vertex_buffer_delete(line_buffer);  line_buffer  = NULL;
    vertex_buffer_delete(point_buffer); point_buffer = NULL;

    if(shader)
        glDeleteProgram(shader);
    if(text_shader)
        glDeleteProgram(text_shader);

    shader = text_shader = 0;
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

    window = glfwCreateWindow( width, height, argv[0], NULL, NULL );

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
    reshape( window, width, height );

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

    glDeleteProgram( text_shader );
    glDeleteProgram( shader );

    glfwDestroyWindow( window );
    glfwTerminate( );

    return EXIT_SUCCESS;
}
#endif  // __PS4__ disable on PS4