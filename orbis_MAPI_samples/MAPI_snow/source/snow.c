/*****************************************************************************
 * ==> Snow demo ------------------------------------------------------------*
 *****************************************************************************
 * Description : Snow particle system demo                                   *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2017, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

// std
#include <math.h>
#include <stdlib.h>

// mini API
#include "MiniCommon.h"
#include "MiniGeometry.h"
#include "MiniVertex.h"
#include "MiniShapes.h"
#include "MiniParticles.h"
#include "MiniShader.h"
#include "MiniRenderer.h"


// function prototypes
void CreateViewport(float w, float h);

//------------------------------------------------------------------------------
MINI_Shader       g_Shader;
GLuint            g_ShaderProgram    = 0;
float*            g_pVertexBuffer    = 0;
unsigned          g_VertexCount      = 0;
float             g_Radius           = 0.02f;
float             g_Angle            = 0.0f;
float             g_RotationSpeed    = 0.1f;
const unsigned    g_ParticleCount    = 200;
int               g_Initialized      = 0;
int               g_SceneInitialized = 0;
MINI_Particles    g_Particles;
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
    srand((unsigned)time(0));

    g_Particles.m_Count = 0;

    // compile, link and use shader
    g_ShaderProgram = miniCompileShaders(miniGetVSColored(), miniGetFSColored());
    glUseProgram(g_ShaderProgram);

    // get shader attributes
    g_Shader.m_VertexSlot = glGetAttribLocation(g_ShaderProgram, "mini_vPosition");
    g_Shader.m_ColorSlot  = glGetAttribLocation(g_ShaderProgram, "mini_vColor");

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
    g_VertexFormat.m_UseTextures = 0;
    g_VertexFormat.m_UseColors   = 1;

    // generate disk
    miniCreateDisk(0.0f,
                   0.0f,
                   g_Radius,
                   5,
                   0xFFFFFFFF,
                  &g_VertexFormat,
                  &g_pVertexBuffer,
                  &g_VertexCount);

    g_SceneInitialized = 1;
}
//------------------------------------------------------------------------------
void DeleteScene()
{
    g_SceneInitialized = 0;

    miniClearParticles(&g_Particles);

    // delete vertices
    if (g_pVertexBuffer)
    {
        free(g_pVertexBuffer);
        g_pVertexBuffer = 0;
    }

    // delete shader program
    if (g_ShaderProgram)
        glDeleteProgram(g_ShaderProgram);

    g_ShaderProgram = 0;
}

struct timeval  t1, t2;
static float elapsedTime;
//------------------------------------------------------------------------------
void UpdateScene(float elapsedTime2)
{
    // timing
    gettimeofday( &t2, NULL );
    elapsedTime = t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6;
    t1 = t2;

    //debugNetPrintf(DEBUG,"%f\n", elapsedTime);

    unsigned       i;
    MINI_Vector3   startPos;
    MINI_Vector3   startDir;
    MINI_Vector3   startVelocity;
    MINI_Particle* pNewParticle;

    startPos.m_X      =  0.0f; // between -2.2 to 2.2
    startPos.m_Y      =  2.0f;
    startPos.m_Z      = -3.0f; // between -1.0 to -5.0
    startDir.m_X      =  1.0f;
    startDir.m_Y      = -1.0f;
    startDir.m_Z      =  0.0f;
    startVelocity.m_X =  0.0f;
    startVelocity.m_Y =  0.05f;
    startVelocity.m_Z =  0.0f;

    // iterate through particles to update
    for (i = 0; i < g_ParticleCount; ++i)
    {
        // emit a new particle
        if ((pNewParticle = miniEmitParticle(&g_Particles,
                                            &startPos,
                                            &startDir,
                                            &startVelocity,
                                            g_ParticleCount)))
        {
            // initialize default values
            pNewParticle->m_Position.m_X =  ((random() % 44) - 22.0f) * 0.1f; // between -2.2 to  2.2
            pNewParticle->m_Position.m_Z = -((random() % 40) + 10.0f) * 0.1f; // between -1.0 to -5.0
            pNewParticle->m_Velocity.m_X =  ((random() % 4)  - 2.0f)  * 0.01f;
            pNewParticle->m_Velocity.m_Y =  ((random() % 4)  + 2.0f)  * 0.01f;

            // select a random start height the first time particles are emitted
            if (!g_Initialized)
                pNewParticle->m_Position.m_Y = 2.0f + ((random() % 200) * 0.01f);
        }

        // no particles to show? (e.g all were removed in this loop)
        if (!g_Particles.m_Count)
            continue;

        // move particle
        if (i >= g_Particles.m_Count)
            miniMoveParticle(&g_Particles.m_pParticles[g_Particles.m_Count - 1],
                              elapsedTime * 20.0f);
        else
            miniMoveParticle(&g_Particles.m_pParticles[i], elapsedTime * 20.0f);

        // is particle out of screen?
        if (g_Particles.m_pParticles[i].m_Position.m_Y <= -2.0f ||
            g_Particles.m_pParticles[i].m_Position.m_X <= -4.0f ||
            g_Particles.m_pParticles[i].m_Position.m_X >=  4.0f)
        {
            // delete it from system
            miniDeleteParticle(&g_Particles, i);
            continue;
        }
    }

    g_Initialized = 1;
}
//------------------------------------------------------------------------------
void DrawScene()
{
    unsigned     i;
    MINI_Vector3 t;
    MINI_Matrix  modelViewMatrix;

    miniBeginScene(0.1f, 0.35f, 0.66f, 1.0f);

    // iterate through particles to draw
    for (i = 0; i < g_Particles.m_Count; ++i)
    {
        // set translation
        t.m_X = g_Particles.m_pParticles[i].m_Position.m_X;
        t.m_Y = g_Particles.m_pParticles[i].m_Position.m_Y;
        t.m_Z = g_Particles.m_pParticles[i].m_Position.m_Z;

        miniGetTranslateMatrix(&t, &modelViewMatrix);

        // connect model view matrix to shader
        GLint modelviewUniform = glGetUniformLocation(g_ShaderProgram, "mini_uModelview");
        glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

        // draw the particle
        miniDrawDisk(g_pVertexBuffer, g_VertexCount, &g_VertexFormat, &g_Shader);
    }

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
    wcex.lpszClassName = "MAPI_snow";

    if (!RegisterClassEx(&wcex))
        return 0;

    // create the main window
    hWnd = CreateWindowEx(0,
                          "MAPI_snow",
                          "MiniAPI Snow Particles Demo",
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
