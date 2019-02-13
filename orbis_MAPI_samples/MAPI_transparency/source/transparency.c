/*****************************************************************************
 * ==> Transparency demo ----------------------------------------------------*
 *****************************************************************************
 * Description : A transparency effect, press the left/right arrow keys to   *
 *               increase or decrease the rotation speed, press the up/down  *
 *               arrow keys to increase or decrease the transparency         *
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
#include "MiniVertex.h"
#include "MiniShapes.h"
#include "MiniShader.h"
#include "MiniRenderer.h"

#define GLASS_TEXTURE_FILE "host0:frosted_glass.png"
#define CLOUD_TEXTURE_FILE "host0:cloud.png"

//#define GLASS_TEXTURE_FILE "..\\..\\..\\Resources\\frosted_glass.bmp"
//#define CLOUD_TEXTURE_FILE "..\\..\\..\\Resources\\cloud.bmp"

// function prototypes
void CreateViewport(float w, float h);

//------------------------------------------------------------------------------
MINI_Shader       g_Shader;
GLuint            g_ShaderProgram      = 0;
float*            g_pSphereVB          = 0;
unsigned int      g_SphereVertexCount  = 0;
MINI_Index*       g_pSphereIndexes     = 0;
unsigned int      g_SphereIndexCount   = 0;
float*            g_pSurfaceVB         = 0;
unsigned int      g_SurfaceVertexCount = 0;
const float       g_SurfaceWidth       = 20.0f;
const float       g_SurfaceHeight      = 20.0f;
const float       g_SphereRadius       = 1.0f;
float             g_Angle              = 0.0f;
float             g_RotationSpeed      = 0.1f;
float             g_AlphaLevel         = 0.5f;
int               g_SceneInitialized   = 0;
GLuint            g_GlassTextureIndex  = GL_INVALID_VALUE;
GLuint            g_CloudTextureIndex  = GL_INVALID_VALUE;
GLuint            g_TexSamplerSlot     = 0;
GLuint            g_AlphaSlot          = 0;
GLuint            g_ModelviewUniform   = 0;
MINI_VertexFormat g_VertexFormat;
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
            switch (wParam)
            {
                case VK_LEFT:  g_RotationSpeed -= 0.005; break;
                case VK_RIGHT: g_RotationSpeed += 0.005; break;
                case VK_UP:    g_AlphaLevel    += 0.005; break;
                case VK_DOWN:  g_AlphaLevel    -= 0.005; break;
            }

            // limit alpha level to min and max bounds
            if (g_AlphaLevel > 1.0f)
                g_AlphaLevel = 1.0f;
            else
            if (g_AlphaLevel < 0.0f)
                g_AlphaLevel = 0.0f;

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
void CreateViewport(float w, float h)
{
    // calculate matrix items
    const float zNear  = 1.0f;
    const float zFar   = 20.0f;
    const float fov    = 45.0f;
    const float aspect = w / h;

    // create the OpenGL viewport
    glViewport(0, 0, w, h);

    MINI_Matrix matrix;
    miniGetPerspective(&fov, &aspect, &zNear, &zFar, &matrix);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(g_ShaderProgram, "mini_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &matrix.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void InitScene(int w, int h)
{
    // compile, link and use shader
    g_ShaderProgram = miniCompileShaders(miniGetVSTexAlpha(), miniGetFSTexAlpha());
    glUseProgram(g_ShaderProgram);

    // get shader attributes
    g_Shader.m_VertexSlot   = glGetAttribLocation(g_ShaderProgram,  "mini_vPosition");
    g_Shader.m_ColorSlot    = glGetAttribLocation(g_ShaderProgram,  "mini_vColor");
    g_Shader.m_TexCoordSlot = glGetAttribLocation(g_ShaderProgram,  "mini_vTexCoord");
    g_TexSamplerSlot        = glGetAttribLocation(g_ShaderProgram,  "mini_sColorMap");
    g_AlphaSlot             = glGetUniformLocation(g_ShaderProgram, "mini_uAlpha");
    g_ModelviewUniform      = glGetUniformLocation(g_ShaderProgram, "mini_uModelview");

    // create the viewport
    CreateViewport(w, h);

    g_VertexFormat.m_UseNormals  = 0;
    g_VertexFormat.m_UseTextures = 1;
    g_VertexFormat.m_UseColors   = 1;

    // generate surface
    miniCreateSurface(&g_SurfaceWidth,
                      &g_SurfaceHeight,
                      0xFFFFFFFF,
                      &g_VertexFormat,
                      &g_pSurfaceVB,
                      &g_SurfaceVertexCount);

    // generate sphere
    miniCreateSphere(&g_SphereRadius,
                     10,
                     24,
                     0xFFFFFFFF,
                     &g_VertexFormat,
                     &g_pSphereVB,
                     &g_SphereVertexCount,
                     &g_pSphereIndexes,
                     &g_SphereIndexCount);

    // load textures
    g_GlassTextureIndex = load_png_asset_into_texture(GLASS_TEXTURE_FILE);
    g_CloudTextureIndex = load_png_asset_into_texture(CLOUD_TEXTURE_FILE);

    debugNetPrintf(DEBUG, "load_png_asset_into_texture ret: %d\n", g_GlassTextureIndex);
	
	debugNetPrintf(DEBUG, "load_png_asset_into_texture ret: %d\n", g_CloudTextureIndex);
	
    g_SceneInitialized = 1;
}
//------------------------------------------------------------------------------
void DeleteScene()
{
    g_SceneInitialized = 0;

    // delete sphere buffer index table
    if (g_pSphereIndexes)
    {
        free(g_pSphereIndexes);
        g_pSphereIndexes = 0;
    }

    // delete sphere vertices
    if (g_pSphereVB)
    {
        free(g_pSphereVB);
        g_pSphereVB = 0;
    }

    // delete surface vertices
    if (g_pSurfaceVB)
    {
        free(g_pSurfaceVB);
        g_pSurfaceVB = 0;
    }

    if (g_GlassTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &g_GlassTextureIndex);

    g_GlassTextureIndex = GL_INVALID_VALUE;

    if (g_CloudTextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &g_CloudTextureIndex);

    g_CloudTextureIndex = GL_INVALID_VALUE;

    // delete shader program
    if (g_ShaderProgram)
        glDeleteProgram(g_ShaderProgram);

    g_ShaderProgram = 0;
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
 
    elapsedTime = (float)track;
	
    // calculate next rotation angle
    g_Angle += (g_RotationSpeed * elapsedTime * 10.0f);

    // is rotating angle out of bounds?
    while (g_Angle >= 6.28f)
        g_Angle -= 6.28f;
}
//------------------------------------------------------------------------------
void DrawScene()
{
    float        xAngle;
    MINI_Vector3 t;
    MINI_Vector3 r;
    MINI_Matrix  translateMatrix;
    MINI_Matrix  xRotateMatrix;
    MINI_Matrix  yRotateMatrix;
    MINI_Matrix  modelViewMatrix;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // configure OpenGL to draw opaque objects (NOTE all opaque object should be drawn before,
    // because depth test should be deactivated later to allow alpha blending to work correctly)
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);

    // populate surface translation vector
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -10.0f;

    // get translation matrix
    miniGetTranslateMatrix(&t, &modelViewMatrix);

    // connect model view matrix to shader
    glUniformMatrix4fv(g_ModelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // configure texture to draw
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(g_TexSamplerSlot, GL_TEXTURE0);

    // bind cloud texture
    glBindTexture(GL_TEXTURE_2D, g_CloudTextureIndex);

    // set alpha transparency level to draw surface (NOTE surface doesn't use transparency)
    glUniform1f(g_AlphaSlot, 1.0f);

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    // draw background image
    miniDrawSurface(g_pSurfaceVB,
                    g_SurfaceVertexCount,
                    &g_VertexFormat,
                    &g_Shader);

    // bind glass texture
    glBindTexture(GL_TEXTURE_2D, g_GlassTextureIndex);

    // populate sphere translation vector
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -4.0f;

    // get translation matrix
    miniGetTranslateMatrix(&t, &translateMatrix);

    // set rotation on X axis
    r.m_X = 1.0f;
    r.m_Y = 0.0f;
    r.m_Z = 0.0f;

    // rotate 90 degrees
    xAngle = 1.57075;

    // calculate model view matrix (it's a rotation on the y axis)
    miniGetRotateMatrix(&xAngle, &r, &xRotateMatrix);

    // set rotation on Y axis
    r.m_X = 0.0f;
    r.m_Y = 1.0f;
    r.m_Z = 0.0f;

    // calculate model view matrix (it's a rotation on the y axis)
    miniGetRotateMatrix(&g_Angle, &r, &yRotateMatrix);

    // build model view matrix
    miniMatrixMultiply(&xRotateMatrix,   &yRotateMatrix,   &modelViewMatrix);
    miniMatrixMultiply(&modelViewMatrix, &translateMatrix, &modelViewMatrix);

    // connect model view matrix to shader
    glUniformMatrix4fv(g_ModelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // set alpha transparency level to draw sphere
    glUniform1f(g_AlphaSlot, g_AlphaLevel);

    // configure OpenGL to draw transparency (NOTE all opaque objects should be drawn before,
    // because depth test should be disabled to allow alpha blending to work correctly)
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // draw the transparent sphere
    miniDrawSphere(g_pSphereVB,
                   g_SphereVertexCount,
                   g_pSphereIndexes,
                   g_SphereIndexCount,
                   &g_VertexFormat,
                   &g_Shader);

    miniEndScene();
}

void
pad_special(int special)
{
   switch (special) {
      case 0: //_KEY_LEFT:
         g_RotationSpeed -= 0.005;
         break;
      case 1: //_KEY_RIGHT:
         g_RotationSpeed += 0.005;
         break;
      case 2: //_KEY_UP:
         g_AlphaLevel -= 0.005;
         break;
      case 3: //_KEY_DOWN:
         g_AlphaLevel += 0.005;
         break;
   }
}

//------------------------------------------------------------------------------

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
    wcex.lpszClassName = "MAPI_transparency";

    if (!RegisterClassEx(&wcex))
        return 0;

    // create the main window
    hWnd = CreateWindowEx(0,
                          "MAPI_transparency",
                          "MiniAPI Transparency Demo",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          400,
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
