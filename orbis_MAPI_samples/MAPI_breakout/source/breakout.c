/*****************************************************************************
 * ==> Breakout game demo ---------------------------------------------------*
 *****************************************************************************
 * Description : A simple breakout game                                      *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2017, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

// std
#include <math.h>

// mini API
#include "MiniCommon.h"
#include "MiniGeometry.h"
#include "MiniCollision.h"
#include "MiniVertex.h"
#include "MiniShapes.h"
#include "MiniShader.h"
#include "MiniRenderer.h"
//#include "MiniPlayer.h"



#define BALL_REBOUND_SOUND_FILE "..\\..\\..\\Resources\\ball_rebound.wav"
#define BAR_EXPLODE_SOUND_FILE  "..\\..\\..\\Resources\\bar_explode.wav"

// function prototypes
void CreateViewport(float w, float h);

//------------------------------------------------------------------------------
#define M_BALL_VELOCITY_X 0.005f
#define M_BALL_VELOCITY_Y 0.005f
//------------------------------------------------------------------------------
typedef struct
{
    float m_Left;
    float m_Right;
    float m_Top;
    float m_Bottom;
    float m_BarY;
    float m_Width;
} MINI_Screen;
//------------------------------------------------------------------------------
typedef struct
{
    MINI_Rect    m_Geometry;
    MINI_Vector2 m_L;
    MINI_Vector2 m_R;
    int          m_Exploding;
    MINI_Vector2 m_ExpLOffset;
    MINI_Vector2 m_ExpROffset;
    //ALuint       m_BufferID;
    //ALuint       m_SoundID;
} MINI_Bar;
//------------------------------------------------------------------------------
typedef struct
{
    MINI_Circle  m_Geometry;
    MINI_Vector2 m_Offset;
    MINI_Vector2 m_Inc;
    MINI_Vector2 m_Max;
    //ALuint       m_BufferID;
    //ALuint       m_SoundID;
} MINI_Ball;
//------------------------------------------------------------------------------
typedef struct
{
    MINI_Rect m_Geometry;
    int       m_Visible;
} MINI_Block;
//------------------------------------------------------------------------------
const int g_Level1[91] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
	0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0,  
    0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0,  
    0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0,  
    0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0,  
    0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0,  
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
};
//------------------------------------------------------------------------------
const int g_Level2[91] =
{
    1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1,  
	0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0,  
    0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0,  
    0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0,  
    0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0,  
    0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0,  
    1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 
};
//------------------------------------------------------------------------------
const int g_Level3[91] =
{
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1,
    1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1,
    1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1,
    1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1,
    1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};
//------------------------------------------------------------------------------
const int g_Level4[91] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
	0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0,  
    0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0,  
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,  
    0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0,  
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
};
//------------------------------------------------------------------------------
const int g_Level5[91] =
{
    1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1,  
	0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0,  
    0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0,  
    1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1,  
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,  
    0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0,  
    1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 
};
//------------------------------------------------------------------------------
MINI_Shader        g_Shader;
MINI_Screen        g_Screen;
MINI_Ball          g_Ball;
MINI_Bar           g_Bar;
MINI_Block         g_Blocks[91];
float*             g_pBarVertices       = 0;
unsigned           g_BarVerticesCount   = 0;
float*             g_pBlockVertices     = 0;
unsigned           g_BlockVerticesCount = 0;
float*             g_pBallVertices      = 0;
unsigned           g_BallVerticesCount  = 0;
int                g_BlockColumns       = 13;
int                g_BlockLines         = 7;
int                g_Level              = 0;
int                g_SceneInitialized   = 0;
//static ALCdevice*  g_pOpenALDevice      = 0;
//static ALCcontext* g_pOpenALContext     = 0;
MINI_VertexFormat  g_VertexFormat;
GLuint             g_ShaderProgram;
//------------------------------------------------------------------------------

#if 0

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
            break;

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
        {
            float velocity = 0.0f;

            // is bar exploding?
            if (g_Bar.m_Exploding)
                break;

            switch (wParam)
            {
                case VK_LEFT:  velocity = -0.05f; break;
                case VK_RIGHT: velocity =  0.05f; break;
            }

            // calculate bar next position
            g_Bar.m_Geometry.m_Pos.m_X += velocity;

            // is bar out of bounds?
            if (g_Bar.m_Geometry.m_Pos.m_X >= g_Screen.m_Right - (g_Bar.m_Geometry.m_Size.m_Width * 0.5f))
                g_Bar.m_Geometry.m_Pos.m_X = g_Screen.m_Right - (g_Bar.m_Geometry.m_Size.m_Width * 0.5f);
            else
            if (g_Bar.m_Geometry.m_Pos.m_X <= g_Screen.m_Left + (g_Bar.m_Geometry.m_Size.m_Width * 0.5f))
                g_Bar.m_Geometry.m_Pos.m_X = g_Screen.m_Left + (g_Bar.m_Geometry.m_Size.m_Width * 0.5f);

            break;
        }

        case WM_MOUSEMOVE:
        {
            // is bar exploding?
            if (g_Bar.m_Exploding)
                break;

            const float x = GET_X_LPARAM(lParam);

            RECT clientRect;
            GetClientRect(hWnd, &clientRect);

            // calculate bar next position (NOTE use client height, because the viewport is always uses the
            // height as reference)
            g_Bar.m_Geometry.m_Pos.m_X = g_Screen.m_Left + (x / (float)(clientRect.bottom - clientRect.top));

            // is bar out of bounds?
            if (g_Bar.m_Geometry.m_Pos.m_X >= g_Screen.m_Right - (g_Bar.m_Geometry.m_Size.m_Width * 0.5f))
                g_Bar.m_Geometry.m_Pos.m_X = g_Screen.m_Right - (g_Bar.m_Geometry.m_Size.m_Width * 0.5f);
            else
            if (g_Bar.m_Geometry.m_Pos.m_X <= g_Screen.m_Left + (g_Bar.m_Geometry.m_Size.m_Width * 0.5f))
                g_Bar.m_Geometry.m_Pos.m_X = g_Screen.m_Left + (g_Bar.m_Geometry.m_Size.m_Width * 0.5f);

            break;
        }

        case WM_SIZE:
        {
            if (!g_SceneInitialized)
                break;

            const int width  = ((int)(short)LOWORD(lParam));
            const int height = ((int)(short)HIWORD(lParam));

            CreateViewport(width, height);
            break;
        }

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}
//------------------------------------------------------------------------------
void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    // get the device context (DC)
    *hDC = GetDC(hwnd);

    // set the pixel format for the DC
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize      = sizeof(pfd);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 32;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    // create and enable the render context (RC)
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
}
//------------------------------------------------------------------------------
void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

#endif

//------------------------------------------------------------------------------
void GetScreen(float width, float height, MINI_Screen* pScreen)
{
    // transform the width and height to keep the correct aspect ratio
    width  /= height;
    height /= height;

    // calculate the screen bounds (in the OpenGL view)
    pScreen->m_Left   = -(width  * 0.5f);
    pScreen->m_Right  =  (width  * 0.5f);
    pScreen->m_Top    =  (height * 0.5f);
    pScreen->m_Bottom = -(height * 0.5f);

    // calculate bar start y position
    pScreen->m_BarY = pScreen->m_Bottom + 0.05f;
}



//------------------------------------------------------------------------------
void CreateViewport(float w, float h)
{
    const float       zNear = -1.0f;
    const float       zFar  =  1.0f;
          MINI_Matrix ortho;

    // configure screen
    GetScreen(w, h, &g_Screen);

    // create the OpenGL viewport
    glViewport(0, 0, w, h);

    // get orthogonal projection matrix
    miniGetOrtho(&g_Screen.m_Left,
                 &g_Screen.m_Right,
                 &g_Screen.m_Bottom,
                 &g_Screen.m_Top,
                 &zNear,
                 &zFar,
                 &ortho);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(g_ShaderProgram, "mini_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &ortho.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void InitScene(int w, int h)
{
    int            i;
    int            j;
    int            index;
    float          blockWidth;
    float          blockTop;
    float          surfaceWidth;
    float          surfaceHeight;
    unsigned int   ballSoundFileLen;
    unsigned int   barSoundFileLen;
    unsigned char* pBallSndBuffer;
    unsigned char* pBarSndBuffer;

    // compile, link and use shader
    g_ShaderProgram = miniCompileShaders(miniGetVSColored(), miniGetFSColored());
    glUseProgram(g_ShaderProgram);

    // get vertex and color slots
    g_Shader.m_VertexSlot = glGetAttribLocation(g_ShaderProgram, "mini_vPosition");
    g_Shader.m_ColorSlot  = glGetAttribLocation(g_ShaderProgram, "mini_vColor");

    // create the viewport and configure the screen
    CreateViewport(w, h);

    // initialize ball data
    g_Ball.m_Geometry.m_Pos.m_X = 0.0f;
    g_Ball.m_Geometry.m_Pos.m_Y = 0.0f;
    g_Ball.m_Geometry.m_Radius  = 0.015f;
    g_Ball.m_Offset.m_X         = M_BALL_VELOCITY_X;
    g_Ball.m_Offset.m_Y         = M_BALL_VELOCITY_Y;
    g_Ball.m_Inc.m_X            = 0.001;
    g_Ball.m_Inc.m_Y            = 0.0015f;
    g_Ball.m_Max.m_X            = 0.3f;
    g_Ball.m_Max.m_Y            = 0.3f;
    //g_Ball.m_SoundID            = M_OPENAL_ERROR_ID;

    // initialize bar data
    g_Bar.m_Geometry.m_Pos.m_X       = 0.0f; //2.0 take the bar and moves it off the screen to the right
    g_Bar.m_Geometry.m_Pos.m_Y       = g_Screen.m_Bottom + g_Ball.m_Max.m_Y;
    g_Bar.m_Geometry.m_Size.m_Width  = 0.55f; // this is only the data width of the bar not color
    g_Bar.m_Geometry.m_Size.m_Height = 0.03f; // this is only the data height of the bar not color
    g_Bar.m_R.m_X                    = 0.0f;
    g_Bar.m_R.m_Y                    = 0.0f;
    g_Bar.m_L.m_X                    = 0.0f;
    g_Bar.m_L.m_Y                    = 0.0f;
    g_Bar.m_ExpROffset.m_X           = 0.01f;
    g_Bar.m_ExpROffset.m_Y           = 0.02f;
    g_Bar.m_ExpLOffset.m_X           = 0.04f;
    g_Bar.m_ExpLOffset.m_Y           = 0.015f;
    //g_Bar.m_SoundID                  = M_OPENAL_ERROR_ID;

    // calculate block width (block width * block count + inter space width * block count - 1)
    blockWidth = (g_BlockColumns * 0.06f) + ((g_BlockColumns - 1) * 0.0075f);
    blockTop   =  g_Screen.m_Top - 0.15f;

    // iterate through block lines
    for (j = 0; j < g_BlockLines; ++j)
    {
        // iterate through block columns
        for (i = 0; i < g_BlockColumns; ++i)
        {
            index = (j * g_BlockColumns) + i;

            g_Blocks[index].m_Geometry.m_Pos.m_X       = -(blockWidth * 0.5f) + 0.03f + (i * 0.065f);
            g_Blocks[index].m_Geometry.m_Pos.m_Y       =   blockTop - (j * 0.045f);
            g_Blocks[index].m_Geometry.m_Size.m_Width  =   0.06;
            g_Blocks[index].m_Geometry.m_Size.m_Height =   0.0125f;
            g_Blocks[index].m_Visible                  =   1;
        }
    }

    // populate vertex format
    g_VertexFormat.m_UseNormals  = 0;
    g_VertexFormat.m_UseTextures = 0;
    g_VertexFormat.m_UseColors   = 1;

    // generate disk to draw
    miniCreateDisk(0.0f,
                   0.0f,
                   g_Ball.m_Geometry.m_Radius,
                   30, //how big the ball is?
                   0xFFFF00FF,
                  &g_VertexFormat,
                  &g_pBallVertices,
                  &g_BallVerticesCount);

    surfaceWidth  = 0.55f; //change the width of the bar color only see: g_Bar.m_Geometry.m_Size.m_Width
    surfaceHeight = 0.03f; //change the height of the bar color only see: g_Bar.m_Geometry.m_Size.m_Height

    // create a surface for the bar
    miniCreateSurface(&surfaceWidth,
                      &surfaceHeight,
                       0xFF0033FF,
                      &g_VertexFormat,
                      &g_pBarVertices,
                      &g_BarVerticesCount);

    surfaceWidth  = 0.06f;
    surfaceHeight = 0.025f;

    // update some colors
    g_pBarVertices[11] = 0.4f;
    g_pBarVertices[12] = 0.0f;
    g_pBarVertices[18] = 0.2f;
    g_pBarVertices[19] = 0.4f;
    g_pBarVertices[25] = 0.2f;
    g_pBarVertices[26] = 0.4f;

    // create a surface for the blocks
    miniCreateSurface(&surfaceWidth,
                      &surfaceHeight,
                       0x0000FFFF,
                      &g_VertexFormat,
                      &g_pBlockVertices,
                      &g_BlockVerticesCount);

    // update some colors
    g_pBlockVertices[10] = 0.2f;
    g_pBlockVertices[11] = 0.2f;
    g_pBlockVertices[12] = 0.9f;
    g_pBlockVertices[17] = 0.0f;
    g_pBlockVertices[18] = 0.1f;
    g_pBlockVertices[19] = 0.3f;
    g_pBlockVertices[24] = 0.3f;
    g_pBlockVertices[25] = 0.5f;
    g_pBlockVertices[26] = 0.8f;

    //miniInitializeOpenAL(&g_pOpenALDevice, &g_pOpenALContext);

    // get the sound files length
    //ballSoundFileLen = miniGetFileSize(BALL_REBOUND_SOUND_FILE);
    //barSoundFileLen  = miniGetFileSize(BAR_EXPLODE_SOUND_FILE);

    // allocate buffers
    //pBallSndBuffer = (unsigned char*)calloc(ballSoundFileLen, sizeof(unsigned char));
    //pBarSndBuffer  = (unsigned char*)calloc(barSoundFileLen,  sizeof(unsigned char));

    // load ball sound file
    //miniLoadSoundBuffer(BALL_REBOUND_SOUND_FILE,
    //                    ballSoundFileLen,
    //                    &pBallSndBuffer);

    // load bar sound file and get length
    //miniLoadSoundBuffer(BAR_EXPLODE_SOUND_FILE,
    //                    barSoundFileLen,
    //                    &pBarSndBuffer);

    // create ball rebound sound file
    //miniCreateSound(g_pOpenALDevice,
    //                g_pOpenALContext,
    //                pBallSndBuffer,
    //                ballSoundFileLen,
    //                48000);
                    //&g_Ball.m_BufferID,
                    //&g_Ball.m_SoundID);

    // create bar explode sound file
    //miniCreateSound(g_pOpenALDevice,
    //                g_pOpenALContext,
    //                pBarSndBuffer,
    //                barSoundFileLen,
    //                48000);
                    //&g_Bar.m_BufferID,
                    //&g_Bar.m_SoundID);

    // delete ball sound resource
    //if (pBallSndBuffer)
    //    free(pBallSndBuffer);

    // delete bar sound resource
    //if (pBarSndBuffer)
    //    free(pBarSndBuffer);

    g_SceneInitialized = 1;
}
//------------------------------------------------------------------------------
void DeleteScene()
{
    g_SceneInitialized = 0;

    // delete ball vertices
    if (g_pBallVertices)
    {
        free(g_pBallVertices);
        g_pBallVertices = 0;
    }

    // delete bar vertices
    if (g_pBarVertices)
    {
        free(g_pBarVertices);
        g_pBarVertices = 0;
    }

    // delete block vertices
    if (g_pBlockVertices)
    {
        free(g_pBlockVertices);
        g_pBlockVertices = 0;
    }

    // delete shader program
    if (g_ShaderProgram)
        glDeleteProgram(g_ShaderProgram);

    g_ShaderProgram = 0;

    // stop running ball rebound sound, if needed
    //if (miniIsSoundPlaying(g_Ball.m_SoundID))
    //    miniStopSound(g_Ball.m_SoundID);

    // stop running bar explode sound, if needed
    //if (miniIsSoundPlaying(g_Bar.m_SoundID))
    //    miniStopSound(g_Bar.m_SoundID);

    // release OpenAL interface
    //miniReleaseSound(g_Ball.m_BufferID, g_Ball.m_SoundID);
    //miniReleaseSound(g_Bar.m_BufferID,  g_Bar.m_SoundID);
    //miniReleaseOpenAL(g_pOpenALDevice, g_pOpenALContext);
}

// hack: use fixed values, no time
static float track = 0.00005f;
static float step  = 0.00015f;


//------------------------------------------------------------------------------
void UpdateScene(float elapsedTime)
{
	
	track += step;

   // at bounds, flip step sign
   if(track > 0.05f
   || track < 0.0001f)
       { step *= -1.0; }
 
    //elapsedTime = (float)track;
	elapsedTime = 0.1;
	
    int   i;
    int   collisionX;
    int   collisionY;
    int   allBlocksBroken;
    int   blockCount;
    int   rebuildLevel;
    int   doPlaySound;
    float left;
    float right;
    float top;
    float roundedLeft;
    float roundedRight;
    float roundedTop;

    // set bar y position (may change if the screen change)
    g_Bar.m_Geometry.m_Pos.m_Y = g_Screen.m_BarY;

    // is bar exploding?
    if (g_Bar.m_Exploding)
    {
        // move bar polygons
        g_Bar.m_R.m_X += g_Bar.m_ExpROffset.m_X * (elapsedTime * 10.0f);
        g_Bar.m_R.m_Y += g_Bar.m_ExpROffset.m_Y * (elapsedTime * 10.0f);
        g_Bar.m_L.m_X -= g_Bar.m_ExpLOffset.m_X * (elapsedTime * 10.0f);
        g_Bar.m_L.m_Y += g_Bar.m_ExpLOffset.m_Y * (elapsedTime * 10.0f);

        // explosion ends?
        if (g_Bar.m_L.m_Y > g_Screen.m_Top)
        {
            // reset values
            g_Bar.m_R.m_X     = 0.0f;
            g_Bar.m_R.m_Y     = 0.0f;
            g_Bar.m_L.m_X     = 0.0f;
            g_Bar.m_L.m_Y     = 0.0f;
            g_Bar.m_Exploding = 0;

            // reset x offset velocity
            if (g_Ball.m_Offset.m_X < 0.0f)
                g_Ball.m_Offset.m_X = -M_BALL_VELOCITY_X;
            else
                g_Ball.m_Offset.m_X =  M_BALL_VELOCITY_X;

            // reset y offset velocity
            if (g_Ball.m_Offset.m_Y < 0.0f)
                g_Ball.m_Offset.m_Y = -M_BALL_VELOCITY_Y;
            else
                g_Ball.m_Offset.m_Y =  M_BALL_VELOCITY_Y;
        }
    }

    // move ball - speed control
    g_Ball.m_Geometry.m_Pos.m_X += g_Ball.m_Offset.m_X * (elapsedTime * 5.0f);
    g_Ball.m_Geometry.m_Pos.m_Y += g_Ball.m_Offset.m_Y * (elapsedTime * 5.0f);

    collisionX      = 0;
    collisionY      = 0;
    allBlocksBroken = 1;
    blockCount      = g_BlockColumns * g_BlockLines;

    // iterate through blocks, and check each block to find a collision
    for (i = 0; i < blockCount; ++i)
    {
        // ignore broken blocks
        if (!g_Blocks[i].m_Visible)
            continue;

        // at least 1 block is visible
        allBlocksBroken = 0;

        // is ball in collision with block?
        if (miniCircleRectIntersect(&g_Ball.m_Geometry, &g_Blocks[i].m_Geometry))
        {
            // break the block
            g_Blocks[i].m_Visible = 0;

            // rebound happened on the left or right edge?
            if (g_Ball.m_Geometry.m_Pos.m_Y < g_Blocks[i].m_Geometry.m_Pos.m_Y &&
                g_Ball.m_Geometry.m_Pos.m_Y > g_Blocks[i].m_Geometry.m_Pos.m_Y - g_Blocks[i].m_Geometry.m_Size.m_Height)
                collisionX = 1;
            else
                collisionY = 1;
        }
    }

    rebuildLevel = 0;

    // calculate the rounded value of the screen edges, otherwise rounding errors may drive to
    // incorrect collision detections
    left  = g_Screen.m_Left  + (g_Ball.m_Geometry.m_Radius * 2.0f);
    right = g_Screen.m_Right - (g_Ball.m_Geometry.m_Radius * 2.0f);
    top   = g_Screen.m_Top   - (g_Ball.m_Geometry.m_Radius * 2.5f);
    miniRoundToExp(&left,  3, &roundedLeft);
    miniRoundToExp(&right, 3, &roundedRight);
    miniRoundToExp(&top,   3, &roundedTop);

    // edge reached?
    if (g_Ball.m_Geometry.m_Pos.m_X > roundedRight)
    {
        g_Ball.m_Geometry.m_Pos.m_X = roundedRight;
        collisionX                  = 1;

        // to avoid interference, rebuild level only if one edge is reached (thus ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;
    }
    else
    if (g_Ball.m_Geometry.m_Pos.m_X < roundedLeft)
    {
        g_Ball.m_Geometry.m_Pos.m_X = roundedLeft;
        collisionX                  = 1;

        // to avoid interference, rebuild level only if one edge is reached (thus ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;
    }

    // ball was moving down and is colliding with bar?
    if (g_Ball.m_Offset.m_Y < 0.0f && miniCircleRectIntersect(&g_Ball.m_Geometry, &g_Bar.m_Geometry))
    {
        collisionY = 1;

        // can increase x velocity?
        if (g_Ball.m_Offset.m_X < g_Ball.m_Max.m_X)
            // increase x velocity
            if (g_Ball.m_Offset.m_X > 0.0f)
                g_Ball.m_Offset.m_X += g_Ball.m_Inc.m_X;
            else
                g_Ball.m_Offset.m_X -= g_Ball.m_Inc.m_X;

        // can increase y velocity?
        if (g_Ball.m_Offset.m_X < g_Ball.m_Max.m_Y)
            // increase y velocity
            if (g_Ball.m_Offset.m_Y > 0.0f)
                g_Ball.m_Offset.m_Y += g_Ball.m_Inc.m_Y;
            else
                g_Ball.m_Offset.m_Y -= g_Ball.m_Inc.m_Y;

        // to avoid interference, rebuild level only if one edge is reached (thus ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;
    }
    else
    // edge reached?
    if (g_Ball.m_Geometry.m_Pos.m_Y > roundedTop)
    {
        g_Ball.m_Geometry.m_Pos.m_Y = roundedTop;
        collisionY                  = 1;

        // to avoid interference, rebuild level only if one edge is reached (thus ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;
    }
    else
    if (g_Ball.m_Geometry.m_Pos.m_Y <= g_Screen.m_Bottom)
    {
        g_Ball.m_Geometry.m_Pos.m_Y = g_Screen.m_Bottom;
        collisionY                  = 1;

        // to avoid interference, rebuild level only if one edge is reached (thus ball will never be
        // captured inside blocks)
        if (allBlocksBroken)
            rebuildLevel = 1;

        // bottom reached? Game over...
        g_Bar.m_Exploding = 1;

        //miniPlaySound(g_Bar.m_SoundID);
    }

    // rebuild level, if needed
    if (rebuildLevel)
    {
        // go to next level
        ++g_Level;

        // last level reached?
        if (g_Level >= 5)
            g_Level = 0;

        // iterate through blocks to regenerate
        for (i = 0; i < blockCount; ++i)
            switch (g_Level)
            {
                case 0:
                g_Blocks[i].m_Visible = g_Level1[i];
                break;

                case 1:
                g_Blocks[i].m_Visible = g_Level2[i];
                break;

                case 2:
                g_Blocks[i].m_Visible = g_Level3[i];
                break;

                case 3:
                g_Blocks[i].m_Visible = g_Level4[i];
                break;

                case 4:
                g_Blocks[i].m_Visible = g_Level5[i];
                break;
            }
    }

    //doPlaySound = 0;

    // collision on the x axis?
    if (collisionX)
    {
        g_Ball.m_Offset.m_X = -g_Ball.m_Offset.m_X;
        //doPlaySound         =  1;
    }

    // collision on the y axis?
    if (collisionY)
    {
        g_Ball.m_Offset.m_Y = -g_Ball.m_Offset.m_Y;

        // is bar already exploding?
        //if (!g_Bar.m_Exploding)
        //    doPlaySound = 1;
    }

    // play ball rebound sound
    //if (doPlaySound == 1)
    //    miniPlaySound(g_Ball.m_SoundID);
}
//------------------------------------------------------------------------------
void DrawScene()
{
    MINI_Vector3 t;
    MINI_Matrix  modelViewMatrix;
    int          i;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // set rotation axis
    t.m_X = g_Ball.m_Geometry.m_Pos.m_X;
    t.m_Y = g_Ball.m_Geometry.m_Pos.m_Y;
    t.m_Z = 0.0f;

    // calculate model view matrix
    miniGetTranslateMatrix(&t, &modelViewMatrix);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(g_ShaderProgram, "mini_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // draw the ball
    miniDrawDisk(g_pBallVertices, g_BallVerticesCount, &g_VertexFormat, &g_Shader);

    // is bar currently exploding?
    if (!g_Bar.m_Exploding)
    {
        // set bar position
        t.m_X = g_Bar.m_Geometry.m_Pos.m_X;
        t.m_Y = g_Bar.m_Geometry.m_Pos.m_Y;

        // get bar matrix
        miniGetTranslateMatrix(&t, &modelViewMatrix);

        // connect bar model view matrix to shader
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // draw the bar
        miniDrawSurface(g_pBarVertices,
                        g_BarVerticesCount,
                        &g_VertexFormat,
                        &g_Shader);
    }
    else
    {
        // connect vertex buffer slots to shader
        glEnableVertexAttribArray(g_Shader.m_VertexSlot);
        glEnableVertexAttribArray(g_Shader.m_ColorSlot);

        // set bar left vertex position
        t.m_X = g_Bar.m_Geometry.m_Pos.m_X + g_Bar.m_L.m_X;
        t.m_Y = g_Bar.m_Geometry.m_Pos.m_Y + g_Bar.m_L.m_Y;

        // get bar vertex matrix
        miniGetTranslateMatrix(&t, &modelViewMatrix);

        // connect bar model view matrix to shader
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // draw the first polygon composing the broken bar
        miniDrawBuffer(g_pBarVertices,
                       3,
                       E_Triangles,
                       &g_VertexFormat,
                       &g_Shader);

        // set bar right vertex position
        t.m_X = g_Bar.m_Geometry.m_Pos.m_X + g_Bar.m_R.m_X;
        t.m_Y = g_Bar.m_Geometry.m_Pos.m_Y + g_Bar.m_R.m_Y;

        // get bar vertex matrix
        miniGetTranslateMatrix(&t, &modelViewMatrix);

        // connect bar model view matrix to shader
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // draw the second polygon composing the broken bar
        miniDrawBuffer(g_pBarVertices + g_VertexFormat.m_Stride,
                       3,
                       E_Triangles,
                       &g_VertexFormat,
                       &g_Shader);

        // disconnect vertex buffer slots from shader
        glDisableVertexAttribArray(g_Shader.m_VertexSlot);
        glDisableVertexAttribArray(g_Shader.m_ColorSlot);
    }

    // iterate through blocks to draw
    for (i = 0; i < g_BlockColumns * g_BlockLines; ++i)
    {
        // is block visible?
        if (!g_Blocks[i].m_Visible)
            continue;

        // set block position
        t.m_X = g_Blocks[i].m_Geometry.m_Pos.m_X;
        t.m_Y = g_Blocks[i].m_Geometry.m_Pos.m_Y;

        // get block matrix
        miniGetTranslateMatrix(&t, &modelViewMatrix);

        // connect block model view matrix to shader
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // draw the block
        miniDrawSurface(g_pBlockVertices,
                        g_BlockVerticesCount,
                        &g_VertexFormat,
                        &g_Shader);
    }

    miniEndScene();
}
//------------------------------------------------------------------------------

void
pad_special(int special)
{
   switch (special) {
      case 0: //_KEY_LEFT:
         g_Bar.m_Geometry.m_Pos.m_X -= 0.03f;
         break;
      case 1: //_KEY_RIGHT:
         g_Bar.m_Geometry.m_Pos.m_X += 0.03f;
         break;
      case 2: //_KEY_UP:
         //g_PosVelocity -= 0.005f;
         break;
      case 3: //_KEY_DOWN:
         //g_PosVelocity += 0.005f;
         break;
   }
}


#if 0

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    // try to load application icon from resources
    HICON hIcon = (HICON)LoadImage(GetModuleHandle(NULL),
                                   MAKEINTRESOURCE(IDI_MAIN_ICON),
                                   IMAGE_ICON,
                                   16,
                                   16,
                                   0);

    WNDCLASSEX wcex;
    HWND       hWnd;
    HDC        hDC;
    HGLRC      hRC;
    MSG        msg;
    BOOL       bQuit = FALSE;

    // register the window class
    wcex.cbSize        = sizeof(WNDCLASSEX);
    wcex.style         = CS_OWNDC;
    wcex.lpfnWndProc   = WindowProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = 0;
    wcex.hInstance     = hInstance;
    wcex.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hIconSm       = hIcon;
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName  = NULL;
    wcex.lpszClassName = "MAPI_breakout";

    if (!RegisterClassEx(&wcex))
        return 0;

    // create the main window
    hWnd = CreateWindowEx(0,
                          "MAPI_breakout",
                          "MiniAPI Breakout Game Demo",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          300,
                          400,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hWnd, nCmdShow);

    // enable OpenGL for the window
    EnableOpenGL(hWnd, &hDC, &hRC);

    // stop GLEW crashing on OSX :-/
    glewExperimental = GL_TRUE;

    // initialize GLEW
    if (glewInit() != GLEW_OK)
    {
        // shutdown OpenGL
        DisableOpenGL(hWnd, hDC, hRC);

        // destroy the window explicitly
        DestroyWindow(hWnd);

        return 0;
    }

    RECT clientRect;
    GetClientRect(hWnd, &clientRect);

    // initialize the scene
    InitScene(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);

    // initialize the timer
    unsigned __int64 previousTime = GetTickCount();

    // application main loop
    while (!bQuit)
    {
        // check for messages
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // handle or dispatch messages
            if (msg.message == WM_QUIT)
                bQuit = TRUE;
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            // calculate time interval
            const unsigned __int64 now          =  GetTickCount();
            const double           elapsedTime  = (now - previousTime) / 1000.0;
                                   previousTime =  now;

            UpdateScene(elapsedTime);
            DrawScene();

            SwapBuffers(hDC);

            Sleep (1);
        }
    }

    // delete the scene
    DeleteScene();

    // shutdown OpenGL
    DisableOpenGL(hWnd, hDC, hRC);

    // destroy the window explicitly
    DestroyWindow(hWnd);

    return msg.wParam;
}
#endif
//------------------------------------------------------------------------------
