/*****************************************************************************
 * ==> Money demo -----------------------------------------------------------*
 *****************************************************************************
 * Description : A simple disk showing a coin                                *
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
#include "MiniModels.h"
#include "MiniLevel.h"
#include "MiniParticles.h"
#include "MiniShader.h"
#include "MiniRenderer.h"
//#include "MiniPlayer.h"

#define MONEY_TEXTURE_FILE "host0:money.png"
//#define MONEY_TEXTURE_FILE "..\\..\\..\\Resources\\money.bmp"

// function prototypes
void CreateViewport(float w, float h);

//------------------------------------------------------------------------------
MINI_Shader       g_Shader;
GLuint            g_ShaderProgram;
float*            g_pVertexBuffer    = 0;
unsigned          g_VertexCount      = 0;
float             g_Radius           = 1.0f;
float             g_Angle            = 0.0f;
float             g_RotationSpeed    = 0.1f;
GLuint            g_TextureIndex     = GL_INVALID_VALUE;
GLuint            g_TexSamplerSlot   = 0;
int               g_SceneInitialized = 0;
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
    const float zNear  = 0.001f;
    const float zFar   = 1000.0f;
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
    g_ShaderProgram = miniCompileShaders(miniGetVSTextured(), miniGetFSTextured());
    glUseProgram(g_ShaderProgram);

    // get shader attributes
    g_Shader.m_VertexSlot   = glGetAttribLocation(g_ShaderProgram,  "mini_vPosition");
    g_Shader.m_ColorSlot    = glGetAttribLocation(g_ShaderProgram,  "mini_vColor");
    g_Shader.m_TexCoordSlot = glGetAttribLocation(g_ShaderProgram,  "mini_vTexCoord");
    g_TexSamplerSlot        = glGetAttribLocation(g_ShaderProgram,  "mini_sColorMap");

    // create the viewport
    CreateViewport(w, h);

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    g_VertexFormat.m_UseNormals  = 0;
    g_VertexFormat.m_UseTextures = 1;
    g_VertexFormat.m_UseColors   = 1;

    // generate disk
    miniCreateDisk(0.0f,
                   0.0f,
                   g_Radius,
                   48,
                   0xFFFFFFFF,
                  &g_VertexFormat,
                  &g_pVertexBuffer,
                  &g_VertexCount);

    // load texture
    //g_TextureIndex = miniLoadTexture(MONEY_TEXTURE_FILE);

    g_TextureIndex = load_png_asset_into_texture(MONEY_TEXTURE_FILE);
    debugNetPrintf(DEBUG, "load_png_asset_into_texture ret: %d\n", g_TextureIndex);
	
    g_SceneInitialized = 1;
}
//------------------------------------------------------------------------------
void DeleteScene()
{
    g_SceneInitialized = 0;

    // delete vertices
    if (g_pVertexBuffer)
    {
        free(g_pVertexBuffer);
        g_pVertexBuffer = 0;
    }

    if (g_TextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &g_TextureIndex);

    g_TextureIndex = GL_INVALID_VALUE;

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

    // is angle out of bounds?
    while (g_Angle >= 6.28f)
        g_Angle -= 6.28f;
}
//------------------------------------------------------------------------------
void DrawScene()
{
    float        xAngle;
    float        yAngle;
    MINI_Vector3 t;
    MINI_Vector3 r;
    MINI_Matrix  translateMatrix;
    MINI_Matrix  xRotateMatrix;
    MINI_Matrix  yRotateMatrix;
    MINI_Matrix  zRotateMatrix;
    MINI_Matrix  buildMatrix1;
    MINI_Matrix  buildMatrix2;
    MINI_Matrix  modelViewMatrix;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // set translation
    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -4.0f;

    miniGetTranslateMatrix(&t, &translateMatrix);

    // set rotation on X axis
    r.m_X = 1.0f;
    r.m_Y = 0.0f;
    r.m_Z = 0.0f;

    // rotate 45 degrees
    xAngle = -M_PI / 3.0f;

    miniGetRotateMatrix(&xAngle, &r, &xRotateMatrix);

    // set rotation on Y axis
    r.m_X = 0.0f;
    r.m_Y = 1.0f;
    r.m_Z = 0.0f;

    // rotate 30 degrees
    yAngle = M_PI / 12.0f;

    miniGetRotateMatrix(&yAngle, &r, &yRotateMatrix);

    // set rotation on Z axis
    r.m_X = 0.0f;
    r.m_Y = 0.0f;
    r.m_Z = 1.0f;

    miniGetRotateMatrix(&g_Angle, &r, &zRotateMatrix);

    // build model view matrix
    miniMatrixMultiply(&zRotateMatrix, &yRotateMatrix,   &buildMatrix1);
    miniMatrixMultiply(&buildMatrix1,  &xRotateMatrix,   &buildMatrix2);
    miniMatrixMultiply(&buildMatrix2,  &translateMatrix, &modelViewMatrix);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(g_ShaderProgram, "mini_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // configure texture to draw
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(g_TexSamplerSlot, GL_TEXTURE0);

    // bind the texture
    glBindTexture(GL_TEXTURE_2D, g_TextureIndex);

    // draw the disk
    miniDrawDisk(g_pVertexBuffer, g_VertexCount, &g_VertexFormat, &g_Shader);

    miniEndScene();
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
    wcex.lpszClassName = "MAPI_money";

    if (!RegisterClassEx(&wcex))
        return 0;

    // create the main window
    hWnd = CreateWindowEx(0,
                          "MAPI_money",
                          "MiniAPI Money",
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
