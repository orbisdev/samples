/*****************************************************************************
 * ==> Shader viewer --------------------------------------------------------*
 *****************************************************************************
 * Description : A shader viewer tool                                        *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2018, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

// std
#include <stdio.h>
#include <math.h>
#include <sys/stat.h>
#include <stdlib.h>

// mini API
#include "MiniCommon.h"
#include "MiniGeometry.h"
#include "MiniVertex.h"
#include "MiniShapes.h"
#include "MiniShader.h"
#include "MiniRenderer.h"


//pc_MAPI_samples # clang egl.c MAPI_shader_viewer.c -I$PS4SDK/include/MiniAPI -L.. -lMiniAPI -lm -lGL -lEGL -lX11 -D_MAPI_

#include <orbisFile.h>

#define ICHANNEL0_TEXTURE_FILE  "host0:iChannel0.png"

//------------------------------------------------------------------------------
// one _shared_ generic vertex shader program
const char *g_pVertexShader =
    "precision mediump float;"
    "attribute vec3  mini_aPosition;"
    "uniform   float mini_uTime;"
    "uniform   vec2  mini_uResolution;"
    "uniform   vec2  mini_uMouse;"
    "varying   float iTime;"
    "varying   vec2  iResolution;"
    "varying   vec2  iMouse;"
    ""
    "void main(void)"
    "{"
    "    iResolution = mini_uResolution;"
    "    iTime       = mini_uTime;"
    "    iMouse      = mini_uMouse;"
    "    gl_Position = vec4(mini_aPosition, 1.0);"
    "}";

// a first fragment shader (iTime sample)
const char *g_pshadersource1 =
    "precision mediump float; \
     varying float iTime; \
     varying vec2  iResolution; \
     void main() \
     { \
         vec2 uv = gl_FragCoord.xy / iResolution.xy; \
         vec3 col = 0.5 + 0.5*cos(iTime+uv.xyx+vec3(0,2,4)); \
         gl_FragColor = vec4(col,1.0); \
     }";

// a second fragment shader (flower_of_life_RGB)
// (c) 2016 xoihazard
const char *g_pshadersource =
    "precision mediump float;\n"
    "varying float iTime;\n"
    "varying vec2  iResolution;\n"
    "varying vec2  iMouse;\n"
    "\n"
    "#define TWO_PI 6.2831853072\n"
    "#define PI 3.14159265359\n"
    ""
    "const float timeScale = 0.2;"
    "const float displace = 0.01;"
    "const float gridSize = 20.0;"
    "const float wave = 5.0;"
    "const float brightness = 1.5;"
    ""
    "vec2 rotate(in vec2 v, in float angle) {"
    "    float c = cos(angle);"
    "    float s = sin(angle);"
    "    return v * mat2(c, -s, s, c);"
    "}"
    "vec3 coordToHex(in vec2 coord, in float scale, in float angle) {"
    "   vec2 c = rotate(coord, angle);"
    "   float q = (1.0 / 3.0 * sqrt(3.0) * c.x - 1.0 / 3.0 * c.y) * scale;"
    "   float r = 2.0 / 3.0 * c.y * scale;"
    "   return vec3(q, r, -q - r);"
    "}"
    "vec3 hexToCell(in vec3 hex, in float m) {"
    "    return fract(hex / m) * 2.0 - 1.0;"
    "}"
    "float absMax(in vec3 v) {"
    "    return max(max(abs(v.x), abs(v.y)), abs(v.z));"
    "}"
    "float nsin(in float value) {"
    "    return sin(value * TWO_PI) * 0.5 + 0.5;"
    "}"
    "float hexToFloat(in vec3 hex, in float amt) {"
    "    return mix(absMax(hex), 1.0 - length(hex) / sqrt(3.0), amt);"
    "}"
    "float calc(in vec2 tx, in float time) {"
    "    float angle = PI * nsin(time * 0.1) + PI / 6.0;"
    "    float len = 1.0 - length(tx) * nsin(time);"
    "    float value = 0.0;"
    "    vec3 hex = coordToHex(tx, gridSize * nsin(time * 0.1), angle);"
    "    for (int i = 0; i < 3; i++) {"
    "        float offset = float(i) / 3.0;"
    "        vec3 cell = hexToCell(hex, 1.0 + float(i));"
    "        value += nsin(hexToFloat(cell,nsin(len + time + offset)) *" 
    "                  wave * nsin(time * 0.5 + offset) + len + time);"
    "    }"
    "    return value / 3.0;"
    "}"
    "void main(void)"
    "{"
    "    vec2 tx = (gl_FragCoord.xy / iResolution.xy) - 0.5;"
    "    tx.x *= iResolution.x / iResolution.y;"
    "    float time = iTime * timeScale;"
    "    vec3 rgb = vec3(0.0, 0.0, 0.0);"
    "    for (int i = 0; i < 3; i++) {"
    "        float time2 = time + float(i) * displace;"
    "        rgb[i] += pow(calc(tx, time2), 2.0);"
    "    }"
    "    gl_FragColor = vec4(rgb * brightness, 1.0);"
    "}";

//------------------------------------------------------------------------------
#define NUM_OF_PROGRAMS  (2)

MINI_Shader        g_Shader            [NUM_OF_PROGRAMS];
GLuint             g_ShaderProgram     [NUM_OF_PROGRAMS];
MINI_VertexFormat  g_VertexFormat      [NUM_OF_PROGRAMS];
float*             g_pSurfaceVB        [NUM_OF_PROGRAMS];
unsigned int       g_SurfaceVertexCount[NUM_OF_PROGRAMS];

// shared between shaders
const float        g_SurfaceWidth     = 10.0f;
const float        g_SurfaceHeight    = 12.5f;
float              g_Time             = 0.0f;
GLuint             g_TimeSlot         = 0;
GLuint             g_ResolutionSlot   = 0;
MINI_Vector2       g_Resolution;      // dimension (x, y)
GLuint             g_MouseSlot        = 0;
MINI_Vector2       g_MousePos;        // position (x, y)

/// second shader can use iChannel0.png as Texture
GLuint             g_TextureIndex     = GL_INVALID_VALUE;
GLuint             g_TexSamplerSlot   = 0; // iChannel0

int                g_SceneInitialized = 0;
//unsigned long long g_PreviousTime   = 0L;
//------------------------------------------------------------------------------
static void CreateViewport(float w, float h)
{
    // create the OpenGL viewport
    glViewport(0, 0, w, h);

    // set the screen resolution
    g_Resolution.m_X = w;
    g_Resolution.m_Y = h;
}


//------------------------------------------------------------------------------
void InitScene_1(int w, int h)
{
    // we will use pad to hook mouse coord: reset center position
    g_MousePos.m_X = g_MousePos.m_Y = 0.0f;

    for(int i=0; i<NUM_OF_PROGRAMS; i++)
    {
        g_pSurfaceVB[i]         = 0;
        g_SurfaceVertexCount[i] = 0;

        const char *g_frag_source;
        switch(i) // switch fragment program source
        {
          case 0: g_frag_source = g_pshadersource;  break;
          case 1: g_frag_source = g_pshadersource1; break;
        }

        // compile, link and use shader

    /* we can use OrbisGl wrappers, or MiniAPI ones */
    #ifndef _MAPI_
        GLuint vertexShader;
        GLuint fragmentShader;

        // one shared vertex program source
        vertexShader = orbisGlCompileShader(GL_VERTEX_SHADER, g_pVertexShader);
        if (!vertexShader) {
            debugNetPrintf(DEBUG, "Error during compiling vertex shader !\n");
        }
        // different fragment shaders after switch()
        fragmentShader = orbisGlCompileShader(GL_FRAGMENT_SHADER, g_frag_source);
        if (!fragmentShader) {
            debugNetPrintf(DEBUG, "Error during compiling fragment shader !\n");
        }

        g_ShaderProgram[i] = orbisGlLinkProgram(vertexShader, fragmentShader);
        if (!g_ShaderProgram[i]) {
            debugNetPrintf(DEBUG, "Error during linking shader ! program_id=%d (0x%08x)\n", g_ShaderProgram[i], g_ShaderProgram[i]);
        }
        debugNetPrintf(DEBUG, "g_ShaderProgram[%d]:%d\n", i, g_ShaderProgram[i]);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

    #else /* include and links against MiniAPI library! */

        g_ShaderProgram[i] = miniCompileShaders(g_pVertexShader, g_frag_source);

    #endif

        if(!g_ShaderProgram[i]) { sleep(5); }

        glUseProgram(g_ShaderProgram[i]);  // we setup this one

        // get shader attributes
        g_Shader[i].m_VertexSlot = glGetAttribLocation (g_ShaderProgram[i], "mini_aPosition");
        g_TimeSlot               = glGetUniformLocation(g_ShaderProgram[i], "mini_uTime");
        g_ResolutionSlot         = glGetUniformLocation(g_ShaderProgram[i], "mini_uResolution");
        g_MouseSlot              = glGetUniformLocation(g_ShaderProgram[i], "mini_uMouse");
        // for texture
        g_TexSamplerSlot         = glGetAttribLocation( g_ShaderProgram[i], "iChannel0");

        // create the viewport
        CreateViewport(w, h);
        debugNetPrintf(DEBUG, "g_ResolutionSlot: %f %f\n", g_Resolution.m_X, g_Resolution.m_Y);

        // notify shader about screen size
        glUniform2f(g_ResolutionSlot, g_Resolution.m_X, g_Resolution.m_Y);

        // initialize the mouse (or finger) position in the shader
        glUniform2f(g_MouseSlot, 0, 0);

        // configure OpenGL depth testing
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LEQUAL);
        glDepthRangef(0.0f, 1.0f);

        // enable culling
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glFrontFace(GL_CCW);

        g_VertexFormat[i].m_UseNormals  = 0;
        g_VertexFormat[i].m_UseTextures = 0;
        g_VertexFormat[i].m_UseColors   = 0;

        // generate surface
        miniCreateSurface(&g_SurfaceWidth,
                          &g_SurfaceHeight,
                           0xFFFFFFFF,
                          &g_VertexFormat[i],
                          &g_pSurfaceVB[i],
                          &g_SurfaceVertexCount[i]);
        if(i == 1)
        {
            // load iChannel0 texture
            g_TextureIndex = load_png_asset_into_texture(ICHANNEL0_TEXTURE_FILE);
            debugNetPrintf(DEBUG, "load_png_asset_into_texture ret: %d\n", g_TextureIndex);

            glActiveTexture(GL_TEXTURE0);
            glUniform1i(g_TexSamplerSlot, GL_TEXTURE0);
        }
    }
    g_SceneInitialized = 1;
}
//------------------------------------------------------------------------------
void DeleteScene_1()
{
    g_SceneInitialized = 0;
    for(int i=0; i<NUM_OF_PROGRAMS; i++)
    {
        // delete surface vertices
        if (g_pSurfaceVB[i])
        {
            free(g_pSurfaceVB[i]);
            g_pSurfaceVB[i] = 0;
        }

        // iChannel0
        if (g_TextureIndex != GL_INVALID_VALUE)
            glDeleteTextures(1, &g_TextureIndex);

        g_TextureIndex = GL_INVALID_VALUE;

        // delete shader program
        if (g_ShaderProgram[i])
            glDeleteProgram(g_ShaderProgram[i]);

        g_ShaderProgram[i] = 0;
    }
}
//------------------------------------------------------------------------------
static float frame = 0.;
extern float p1_pos_x, p1_pos_y; // from main()
void UpdateScene_1(float elapsedTime)
{
    elapsedTime = frame + 0.05f;

    g_Time += elapsedTime * 0.5f;  // calculate next time

    for(int i=0; i<NUM_OF_PROGRAMS; i++)
    {
        glUseProgram(g_ShaderProgram[i]);

        // notify shader about elapsed time
        glUniform1f(g_TimeSlot, g_Time);

        // report
        #ifdef __PS4__
        // notify shader about mouse position
        glUniform2f(g_MouseSlot, g_MousePos.m_X, g_MousePos.m_Y);
        //debugNetPrintf(DEBUG, "g_Time = %f %f\n", g_Time, sin(g_Time));

        #else
        // notify shader about mouse position
        glUniform2f(g_MouseSlot, p1_pos_x, p1_pos_y);
        //printf("g_Time = %f %f\n", g_Time, sin(g_Time));
        //printf("update_pos: %f %f\n", p1_pos_x, p1_pos_y);

        #endif
    }
}
//------------------------------------------------------------------------------
int scene_num = 0;
void DrawScene_1(void)
{
    int i = scene_num %NUM_OF_PROGRAMS;
    {
        glUseProgram(g_ShaderProgram[i]);

        if(i == 1)
        {
            // refresh state for this shader
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, g_TextureIndex);
        }

        // configure OpenGL depth testing
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LEQUAL);
        glDepthRangef(0.0f, 1.0f);

        // enable culling
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glFrontFace(GL_CCW);

        //miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);// we already clean in main render loop

        // draw the surface on which the shader will be executed
        miniDrawSurface(g_pSurfaceVB[i],
                        g_SurfaceVertexCount[i],
                       &g_VertexFormat[i],
                       &g_Shader[i]);
    }
    //miniEndScene(); // a stub, does nothing
}
//------------------------------------------------------------------------------
#ifdef __PS4__
// hook pad over miniMouse vec2(x, y)
// we get feedback on each keypress
#define STEP  (0.005)  // a small positive delta
void pad_special(int special)
{
    switch (special)
    {
        case 0: //_KEY_LEFT:
            if(g_MousePos.m_X > -1.000) g_MousePos.m_X -= STEP;
            break;
        case 1: //_KEY_RIGHT:
            if(g_MousePos.m_X < 1.000)  g_MousePos.m_X += STEP;
            break;
        case 2: //_KEY_UP:
            if(g_MousePos.m_Y < 1.000)  g_MousePos.m_Y += STEP;
            scene_num++;
            //debugNetPrintf(DEBUG, "scene_num = %d %d\n", scene_num, scene_num %NUM_OF_PROGRAMS);
            sceKernelUsleep(100000);
            break;
        case 3: //_KEY_DOWN:
            if(g_MousePos.m_Y > -1.000) g_MousePos.m_Y -= STEP;
            break;
        /*
        case 4: //_BUTTON_X:
            DeleteScene_1();
            InitScene_1(ATTR_ORBISGL_WIDTH,ATTR_ORBISGL_HEIGHT);
            debugNetPrintf(DEBUG, "Re-Init Scene!\n");
            break;
        */
        default:
            /*do nothing*/
            break;
   }
   debugNetPrintf(DEBUG, "update_pos: %f %f\n", g_MousePos.m_X, g_MousePos.m_Y);
}
#endif
