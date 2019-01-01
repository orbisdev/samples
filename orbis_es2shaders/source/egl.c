/*
 * https://github.com/leokwu/OpenGLES-sample
 * ------------------------------------------
 *
 *
 */

#include <math.h> 
#include <stdlib.h>
#include <stdio.h>
#include <orbisGl.h>
#include <debugnet.h>


const char vertex_src [] =
"                                        \
   attribute vec4        position;       \
   varying mediump vec2  pos;            \
   uniform vec4          offset;         \
                                         \
   void main()                           \
   {                                     \
      gl_Position = position + offset;   \
      pos = position.xy;                 \
   }                                     \
";


const char fragment_src [] =
"                                                      \
   varying mediump vec2    pos;                        \
   uniform mediump float   phase;                      \
                                                       \
   void  main()                                        \
   {                                                   \
      gl_FragColor  =  vec4( 1., 0.9, 0.7, 1.0 ) *     \
        cos( 30.*sqrt(pos.x*pos.x + 1.5*pos.y*pos.y)   \
             + atan(pos.y,pos.x) - phase );            \
   }                                                   \
";
//  some more formulas to play with...
//      cos( 20.*(pos.x*pos.x + pos.y*pos.y) - phase );
//      cos( 20.*sqrt(pos.x*pos.x + pos.y*pos.y) + atan(pos.y,pos.x) - phase );
//      cos( 30.*sqrt(pos.x*pos.x + 1.5*pos.y*pos.y - 1.8*pos.x*pos.y*pos.y)
//            + atan(pos.y,pos.x) - phase );


void
print_shader_info_log (
   GLuint  shader      // handle to the shader
)
{
   GLint  length;

   glGetShaderiv ( shader , GL_INFO_LOG_LENGTH , &length );

   if ( length ) {
      char* buffer  = malloc(length); 
      glGetShaderInfoLog ( shader , length , NULL , buffer );

      free(buffer);

      GLint success;
      glGetShaderiv( shader, GL_COMPILE_STATUS, &success );
      if ( success != GL_TRUE )   exit ( 1 );
   }
}


GLuint
load_shader (
   const char  *shader_source,
   GLenum       type
)
{
   GLuint  shader = glCreateShader( type );

   glShaderSource  ( shader , 1 , &shader_source , NULL );
   glCompileShader ( shader );

   print_shader_info_log ( shader );

   return shader;
}


GLfloat
   norm_x    =  0.0,
   norm_y    =  0.0,
   offset_x  =  0.0,
   offset_y  =  0.0,
   p1_pos_x  =  0.0,
   p1_pos_y  =  0.0;

GLint
   phase_loc,
   offset_loc,
   position_loc;


bool        update_pos = false;

const float vertexArray[] = {
   0.0,  0.5,  0.0,
  -0.5,  0.0,  0.0,
   0.0, -0.5,  0.0,
   0.5,  0.0,  0.0,
   0.0,  0.5,  0.0 
};

//// this is needed for time measuring  -->  frames per second
static struct timezone tz;
static struct timeval t1, t2;
static int num_frames = 0;

void
render(void)
{
   static float  phase = 0;

   //// draw

   glClearColor ( 0.08 , 0.06 , 0.07 , 1.);    // background color
   glClear ( GL_COLOR_BUFFER_BIT );

   glUniform1f ( phase_loc , phase );  // write the value of phase to the shaders phase
    phase  =  fmodf( phase + 0.5f , 2.f * 3.141f );    // and update the local variable
   if ( update_pos ) {  // if the position of the texture has changed due to user action
      GLfloat old_offset_x  =  offset_x;
      GLfloat old_offset_y  =  offset_y;

      offset_x  =  norm_x - p1_pos_x;
      offset_y  =  norm_y - p1_pos_y;

      p1_pos_x  =  norm_x;
      p1_pos_y  =  norm_y;

      offset_x  +=  old_offset_x;
      offset_y  +=  old_offset_y;

      update_pos = false;

      debugNetPrintf(INFO,"offset_x : %.3f, offset_y :%.3f\n", offset_x, offset_y);
   }

   glUniform4f ( offset_loc,  offset_x, offset_y, 0.0, 0.0 );

   glVertexAttribPointer ( position_loc, 3, GL_FLOAT, false, 0, vertexArray );
   glEnableVertexAttribArray ( position_loc );
   glDrawArrays ( GL_TRIANGLE_STRIP, 0, 5 );

   if ( ++num_frames % 1000 == 0 ) {
       gettimeofday( &t2, &tz );
       float dt  =  t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6;
       debugNetPrintf(INFO,"fps: %f\n",num_frames / dt);
       num_frames = 0;
       t1 = t2;
   }
   // we swapbuffer egl in main()
}

void
pad_special(int special)
{
   switch (special) {
      case 0: //_KEY_LEFT:
         offset_x -= 0.05;
         break;
      case 1: //_KEY_RIGHT:
         offset_x += 0.05;
         break;
      case 2: //_KEY_UP:
         offset_y += 0.05;
         break;
      case 3: //_KEY_DOWN:
         offset_y -= 0.05;
         break;
   }
   update_pos = true;
}

////////////////////////////////////////////////////////////////////////////////////////////

static int init()
{
   ///////  the egl part  //////////////////////////////////////////////////////////////////
   //  egl provides an interface to connect the graphics related functionality of openGL ES
   //  with the windowing interface and functionality of the native operation system (X11
   //  was our case.

   //// egl-contexts collect all state descriptions needed required for operation

   //// associate the egl-context with the egl-surface
   //eglMakeCurrent( egl_display, egl_surface, egl_surface, egl_context ); 

   ///////  the openGL part  ///////////////////////////////////////////////////////////////

   GLuint vertexShader   = load_shader ( vertex_src , GL_VERTEX_SHADER  );     // load vertex shader
   GLuint fragmentShader = load_shader ( fragment_src , GL_FRAGMENT_SHADER );  // load fragment shader

   GLuint shaderProgram  = glCreateProgram ();                 // create program object
   glAttachShader ( shaderProgram, vertexShader );             // and attach both...
   glAttachShader ( shaderProgram, fragmentShader );           // ... shaders to it

   glLinkProgram ( shaderProgram );    // link the program
   glUseProgram  ( shaderProgram );    // and select it for usage

   //// now get the locations (kind of handle) of the shaders variables
   position_loc  = glGetAttribLocation  ( shaderProgram , "position" );
   phase_loc     = glGetUniformLocation ( shaderProgram , "phase"    );
   offset_loc    = glGetUniformLocation ( shaderProgram , "offset"   );
   if ( position_loc < 0  ||  phase_loc < 0  ||  offset_loc < 0 ) {
      debugNetPrintf(INFO,"Unable to get uniform location\n");
      return 1;
   }

   gettimeofday ( &t1 , &tz );

   return 0;
}


int
es2sample_init(int argc, char *argv[])
{
    init();

    /* Set initial projection/viewing transformation.
    * We can't be sure we'll get an event when the window
    * first appears.
    */
    glViewport ( 0, 0, ATTR_ORBISGL_WIDTH, ATTR_ORBISGL_HEIGHT);

    return 0;
}
