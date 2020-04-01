/*
  a GLES2 playground on EGL

  2019, 2020, masterzorag
*/

// include a second MAPI scene
//#define MAPI_EARTH    // good
 //#define MAPI_WAVEFRONT  // forces just model, no PS on the bg
 //#define MAPI_4        // broken vert

#include <math.h>
#include <stdbool.h> 
#include <stdlib.h> 
#include <stdio.h>
//#include <time.h>
#include <sys/time.h>
// X11
#include  <X11/Xlib.h>
#include  <X11/Xatom.h>
#include  <X11/Xutil.h>
// GLES2
#include  <GLES2/gl2.h>
#include  <EGL/egl.h>

#include "defines.h" // the parts list


//extern int selected_icon;   // from texture.c
int selected_icon;
int is_facing_left;  // from  sprite.c

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
 
 
Display    *x_display;
Window      win;
EGLDisplay  egl_display;
EGLContext  egl_context;
EGLSurface  egl_surface;
 
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
 
 
bool update_pos = false;
 
const float vertexArray[] = {
   0.0,  0.5,  0.0,
  -0.5,  0.0,  0.0,
   0.0, -0.5,  0.0,
   0.5,  0.0,  0.0,
   0.0,  0.5,  0.0 
};
 
int  num_frames = 0;
//static float frame = 0.0;
 
void render()
{
   static float  phase = 0;
   static int    donesetup = 0;
 
   static XWindowAttributes gwa;
 
   //// draw
 //move on X11 init
   if ( !donesetup ) {
      XWindowAttributes  gwa;
      XGetWindowAttributes ( x_display , win , &gwa );
      glViewport ( 0 , 0 , gwa.width , gwa.height );
      glClearColor ( 0.08 , 0.06 , 0.07 , 1.);    // background color
      //glClearColor(0.5, 0.5, 0.5, 1.0);
      //glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
      donesetup = 1;
   }
// glClear ( GL_COLOR_BUFFER_BIT );
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

#ifdef _BASE_
   glUniform1f ( phase_loc , phase );  // write the value of phase to the shaders phase
   phase = fmodf( phase + 0.5f , 2.f * 3.141f );    // and update the local variable
#endif
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

      printf("update_pos: %f %f\n", p1_pos_x, p1_pos_y);
        
      //on_GLES2_TouchMove((float)norm_x, (float)norm_y, (float)p1_pos_x, (float)p1_pos_y);
   }
 #ifdef _BASE_
   glUniform4f ( offset_loc  ,  offset_x , offset_y , 0.0 , 0.0 );
 
   glVertexAttribPointer ( position_loc, 3, GL_FLOAT, false, 0, vertexArray );
   glEnableVertexAttribArray ( position_loc );
   glDrawArrays ( GL_TRIANGLE_STRIP, 0, 5 );
   
#elif defined _CUBE_
    draw_cube_smooth(frame++);

#elif defined _DEMO_
    if (num_frames %2 == 0)
    {
        on_GLES2_Update(num_frames /2);
    }
    
    #include "texture.h"
    // render all textures VBOs
    for(int i=0; i < NUM_OF_TEXTURES; i++) on_GLES2_Render(i); // skip background

#elif defined _MAPI_

    DrawScene_1(NULL);  // shader_viewer
    UpdateScene_1(0, 0.02);
    UpdateScene_1(1, 0.2);
    
    //usleep(5000);
    #if defined MAPI_EARTH
        // draw using texture (png)
        // like DrawScene program, but with sampler2D
        DrawScene_2();
        UpdateScene_2(0.02);

    #endif

    #if defined MAPI_WAVEFRONT    // wavefront.c
        DrawScene_3();
        UpdateScene_3(0.02);
    #endif
    
    #if defined MAPI_4    // cube-tex.c
        DrawScene_4();
        //UpdateScene_3(0.02);
    #endif

#elif defined PL_MPEG
    // decodes mpeg2 audiovideo
    es2render_pl_mpeg(.03f);

#elif defined _ICONS_
    /// update
    on_GLES2_Update(num_frames);

    // render all textures VBOs
    for(int i=0; i < NUM_OF_TEXTURES; i++) on_GLES2_Render(i); // skip background

#elif defined _SPRITE_
    on_GLES2_Render_sprite(0);
    if( !(num_frames %6) )
        on_GLES2_Update_sprite(num_frames /6);

#elif defined _SOUND_
    on_GLES2_Render_sound(0);
    on_GLES2_Update_sound(num_frames /6);

#endif // now texts

#if defined FT_DEMO
    render_text();   // freetype demo-font.c, renders text just from init_

    render_text_ext(NULL);  // freetype text_ani.c, shared VBO, draw just indexed texts
    es2update_text_ani(1.f);

#elif defined FT_DEMO_2
    // demo-font_2.c init
    render_text();

#endif

#if defined _SOUND_
    render_text2();  // freetype browser.c, have its list
#endif

   // end drawing ops, flip
   eglSwapBuffers ( egl_display, egl_surface );  // get the rendered buffer to the screen
}
 
 
////////////////////////////////////////////////////////////////////////////////////////////
char *shader_file_name;
 
int main(int argc, char **argv)
{
    shader_file_name = argv[1];

   ///////  the X11 part  //////////////////////////////////////////////////////////////////
   // in the first part the program opens a connection to the X11 window manager
   //
 
   x_display = XOpenDisplay ( NULL );   // open the standard display (the primary screen)
   if ( x_display == NULL ) {
      printf("cannot connect to X server\n");
      return 1;
   }
 
   // chensq Window root  =  DefaultRootWindow( x_display );   // get the root window (usually the whole screen)
   Window root = RootWindow(x_display, DefaultScreen(x_display));
   XSetWindowAttributes  swa;
   swa.event_mask  =  ExposureMask | PointerMotionMask | KeyPressMask;
 
   win  =  XCreateWindow (   // create a window with the provided parameters
              x_display, root,
              0, 0, 1024, 480,   0,
              CopyFromParent, InputOutput,
              CopyFromParent, CWEventMask,
              &swa );
   if(!win){
    printf("XCreateWindow error\n");
    return 1;

   }
   XSetWindowAttributes  xattr;
   Atom  atom=None;
   int   one = 1;
 
   xattr.override_redirect = False;
   XChangeWindowAttributes ( x_display, win, CWOverrideRedirect, &xattr );
   atom = XInternAtom ( x_display, "_NET_WM_STATE_FULLSCREEN", True );
   if(atom==None) {
        XSizeHints sizehints;
        sizehints.min_width  = 1024;
        sizehints.min_height = 480;
        sizehints.max_width  = 1024;
        sizehints.max_height = 480;
        sizehints.flags = PMaxSize | PMinSize;
    XSetWMProperties(x_display, win, NULL, NULL,
                         NULL, 0, &sizehints, NULL, NULL);
   }
#if 0
   XChangeProperty (
      x_display, win,
      XInternAtom ( x_display, "_NET_WM_STATE", True ),
      XA_ATOM,  32,  PropModeReplace,
      (unsigned char*) &atom,  1 );
   printf("3333333333\n");
   XChangeProperty (
      x_display, win,
      XInternAtom ( x_display, "_HILDON_NON_COMPOSITED_WINDOW", True ),
      XA_INTEGER,  32,  PropModeReplace,
      (unsigned char*) &one,  1);
#endif
   XMapWindow ( x_display , win );
#if 0
   printf("444444444\n"); 
   XWMHints hints;
   hints.input = True;
   hints.flags = InputHint;
   XSetWMHints(x_display, win, &hints);
 
   XMapWindow ( x_display , win );             // make the window visible on the screen
   XStoreName ( x_display , win , "GL test" ); // give the window a name
 
   //// get identifiers for the provided atom name strings
   Atom wm_state   = XInternAtom ( x_display, "_NET_WM_STATE", False );
   Atom fullscreen = XInternAtom ( x_display, "_NET_WM_STATE_FULLSCREEN", False );
 
   XEvent xev;
   memset ( &xev, 0, sizeof(xev) );
 
   xev.type                 = ClientMessage;
   xev.xclient.window       = win;
   xev.xclient.message_type = wm_state;
   xev.xclient.format       = 32;
   xev.xclient.data.l[0]    = 1;
   xev.xclient.data.l[1]    = fullscreen;
   XSendEvent (                // send an event mask to the X-server
      x_display,
      DefaultRootWindow ( x_display ),
      False,
      SubstructureNotifyMask,
      &xev );
 
 #endif
   ///////  the egl part  //////////////////////////////////////////////////////////////////
   //  egl provides an interface to connect the graphics related functionality of openGL ES
   //  with the windowing interface and functionality of the native operation system (X11
   //  in our case.

   egl_display = eglGetDisplay( (EGLNativeDisplayType) x_display );
   //printf("111111111\n");
   if ( egl_display == EGL_NO_DISPLAY ) {
      printf ("Got no EGL display.\n");
      return 1;
   }
 
   if ( !eglInitialize( egl_display, NULL, NULL ) ) {
      printf ("Unable to initialize EGL\n");
      return 1;
   }
 
   EGLint attr[] = {       // some attributes to set up our egl-interface
      EGL_BUFFER_SIZE, 16,
      EGL_RENDERABLE_TYPE,
      EGL_OPENGL_ES2_BIT,
      EGL_NONE
   };
 
   EGLConfig  ecfg;
   EGLint     num_config;
   if ( !eglChooseConfig( egl_display, attr, &ecfg, 1, &num_config ) ) {
      printf ("Failed to choose config (eglError: \n");
      return 1;
   }
 
   if ( num_config != 1 ) {
      printf("Didn't get exactly one config, but %d\n", num_config);
      return 1;
   }
 
   egl_surface = eglCreateWindowSurface ( egl_display, ecfg, win, NULL );
   if ( egl_surface == EGL_NO_SURFACE ) {
      printf("Unable to create EGL surface\n");
      return 1;
   }
 
   //// egl-contexts collect all state descriptions needed required for operation
   EGLint ctxattr[] = {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
   };
   egl_context = eglCreateContext ( egl_display, ecfg, EGL_NO_CONTEXT, ctxattr );
   if ( egl_context == EGL_NO_CONTEXT ) {
      printf("Unable to create EGL context\n");
      return 1;
   }
 
   //// associate the egl-context with the egl-surface
   eglMakeCurrent( egl_display, egl_surface, egl_surface, egl_context );
 
    const char *gl_exts = (char *) glGetString(GL_EXTENSIONS);
    printf("OpenGL ES 2.x information:\n");
    printf("  version: \"%s\"\n", glGetString(GL_VERSION));
    printf("  shading language version: \"%s\"\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("  vendor: \"%s\"\n", glGetString(GL_VENDOR));
    printf("  renderer: \"%s\"\n", glGetString(GL_RENDERER));
  //printf("  extensions: \"%s\"\n", gl_exts);
    printf("===================================\n");
    
 const float
      window_width  = 1024.0,
      window_height =  480.0;
 
   ///////  the openGL part  ///////////////////////////////////////////////////////////////
   int ret = 0;
   
//goto out;
#ifdef _BASE_
   GLuint vertexShader   = load_shader ( vertex_src , GL_VERTEX_SHADER  );     // load vertex shader
   GLuint fragmentShader = load_shader ( fragment_src , GL_FRAGMENT_SHADER );  // load fragment shader
 
   GLuint shaderProgram  = glCreateProgram ();                 // create program object
   glAttachShader ( shaderProgram, vertexShader   );           // and attach both...
   glAttachShader ( shaderProgram, fragmentShader );           // ... shaders to it
 
   glLinkProgram ( shaderProgram );    // link the program
   glUseProgram  ( shaderProgram );    // and select it for usage
 
   //// now get the locations (kind of handle) of the shaders variables
   position_loc  = glGetAttribLocation  ( shaderProgram , "position" );
   phase_loc     = glGetUniformLocation ( shaderProgram , "phase"    );
   offset_loc    = glGetUniformLocation ( shaderProgram , "offset"   );
   if ( position_loc < 0  ||  phase_loc < 0  ||  offset_loc < 0 ) {
      printf("Unable to get uniform location\n");
      return 1;
   }
#elif defined PL_MPEG
    // demo-font.c init
    es2init_pl_mpeg((int)window_width, (int)window_height);

#elif defined _CUBE_  
    int samples = 0;

    ret = init_cube_smooth((int)window_width, (int)window_height, samples);
    printf("init_cube_smooth: %d\n", ret);

#elif defined _DEMO_  
    on_GLES2_Init((int)window_width, (int)window_height);
    printf("on_GLES2_Init\n");
    // set viewport
    on_GLES2_Size((int)window_width, (int)window_height);
    
#elif defined _MAPI_ 
    // pixelshader sample init: MAPI_shaderviewer.c
    InitScene_1(0, (int)window_width, (int)window_height, NULL, 0);
    InitScene_1(1, (int)window_width, (int)window_height, NULL, 0);
    printf("InitScene\n");

    #if defined MAPI_EARTH
        // texture sample init: MAPI_earth.c
        InitScene_2((int)window_width, (int)window_height);
        printf("InitScene_2\n");
    #endif
    
    #if defined MAPI_WAVEFRONT    
        // wavefront.c
        InitScene_3((int)window_width, (int)window_height);
    #endif
    
    #if defined MAPI_4    // cube-tex.c
        InitScene_4((int)window_width, (int)window_height);
        sleep(2);
    #endif

#elif defined _ICONS_
// sprite.c init
    on_GLES2_Init_icons((int)window_width, (int)window_height);
    printf("on_GLES2_Init\n");
    // set viewport
    on_GLES2_Size_icons((int)window_width, (int)window_height);

#elif defined _SPRITE_
    // sprite.c init
    on_GLES2_Init_sprite((int)window_width, (int)window_height);
    printf("on_GLES2_Init\n");
    // set viewport
    on_GLES2_Size_sprite((int)window_width, (int)window_height);
#endif 

#if defined FT_DEMO
    // demo-font.c init
    es2init_text((int)window_width, (int)window_height);

  #if defined _SOUND_
    on_GLES2_Init_sound((int)window_width, (int)window_height);
    printf("on_GLES2_Init\n");
    // set viewport
    on_GLES2_Size_sound((int)window_width, (int)window_height);

    es2init_browser((int)window_width, (int)window_height); // browser
    //reshape2((int)window_width, (int)window_height);
  #endif
   es2init_text_ani((int)window_width, (int)window_height); // text fx

#elif defined FT_DEMO_2
    // demo-font.c init
    es2init_text((int)window_width, (int)window_height);

#endif

#if defined HAVE_LIBAO
  // simulate sceAudioOut
  ret = orbisAudioInit();                         //printf("ret: %d\n", ret); //1
  ret = orbisAudioInitChannel(0, 1024, 48000, 1); //printf("ret: %d\n", ret); //0
  // start audio play
  orbisAudioResume(0);

#endif

   //// this is needed for time measuring  -->  frames per second
   struct timezone tz;
   struct timeval  t1, t2,
                   t3, t4; // count each second
   gettimeofday ( &t1 , &tz );
   float dt; // delta time
 

   bool quit = false;
   while ( !quit ) { // the main render loop
 
      while ( XPending ( x_display ) ) { // check for events from the x-server
 
         XEvent  xev;
         XNextEvent( x_display, &xev );
 
         if ( xev.type == MotionNotify ) {  // if mouse has moved
          //cout << "move to: << xev.xmotion.x << "," << xev.xmotion.y << endl;
            GLfloat window_y  = (window_height - xev.xmotion.y) - window_height / 2.0;
            norm_y            =  window_y / (window_height / 2.0);
            GLfloat window_x  =  xev.xmotion.x - window_width / 2.0;
            norm_x            =  window_x / (window_width / 2.0);
            update_pos = true;
         }
 
         if ( xev.type == KeyPress ) 
         {  //printf("keycode %d\n", xev.xkey.keycode, xev.xkey.keycode);
            switch(xev.xkey.keycode)
            {
                case 113: printf("Left pressed\n");
                          p1_pos_x -= 0.04, selected_icon--; is_facing_left = 1;
                          break;
                case 114: printf("Right pressed\n");
                          p1_pos_x += 0.04, selected_icon++; is_facing_left = 0;
                          break;
                case 111: printf("Up pressed\n");
                          p1_pos_y += 0.04;
                          break;
                case 116: printf("Down pressed\n");
                          p1_pos_y -= 0.04;
                          break;
                case  39: printf("Square pressed\n");   break;
                case  54: printf("Circle pressed\n");   break;
                case  53: printf("Cross pressed\n");    break;
                case  28: printf("Triangle pressed\n"); break;

                case  24: /* q */ quit = true; break;
                default: break;
            }
         }
      }
 
      render(); // now we finally put something on the screen
 
    // timing
    int n;
    if ( ++num_frames % 100 == 0 ) {
        gettimeofday( &t2, &tz );
        dt = t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6;
        printf("fps: %f, %.4f seconds\n", (float)(num_frames / dt), dt);
        num_frames = 0;
        t1 = t2;
        t4 = t2;
        //usleep(2000);
    }

    if( num_frames > 20 )
    {
        gettimeofday( &t3, &tz );
        dt = t3.tv_sec - t4.tv_sec + (t3.tv_usec - t4.tv_usec) * 1e-6;
        if (dt > 1.f)
        {
            // each passed second...
            printf("dt = %0.4f\n", dt);
            t4 = t3;
            // sample 48000 texture
            //on_GLES2_Update_sound(dt);
        }
    }
      
#if defined _MAPI_ 
      //UpdateScene(dt);
#endif 
      //usleep( 10000 );
   }
 
 
out:

#if defined _DEMO_  
    on_GLES2_Final();

#elif defined PL_MPEG
    es2end_pl_mpeg();

#elif defined _MAPI_ 
    DeleteScene_1(0);
    DeleteScene_1(1);
    DeleteScene_1(2);
    #if defined MAPI_EARTH
        DeleteScene_2();
        
        DeleteScene_3();
    
    #if defined MAPI_4
        DeleteScene_4();
    #endif   
    
    #endif
    //png_test();

#elif defined _SPRITE_  
    on_GLES2_Final_sprite();

#elif defined FT_DEMO
    // FT ones, sorted
    #if defined _SOUND_  
    es2fini_browser();
    #endif
  	es2fini_text_ani();
  	es2sample_end(); // demo-font.c as last one
#endif

#if defined HAVE_LIBAO
  orbisAudioStop();
  orbisAudioFinish();
#endif

   ////  cleaning up...
   eglDestroyContext ( egl_display, egl_context );
   eglDestroySurface ( egl_display, egl_surface );
   eglTerminate      ( egl_display );
   XDestroyWindow    ( x_display, win );
   XCloseDisplay     ( x_display );

   return 0;
}
