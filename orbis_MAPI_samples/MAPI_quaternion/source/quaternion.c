/*****************************************************************************
 * ==> Quaternion demo ------------------------------------------------------*
 *****************************************************************************
 * Description : An animation using a quaternion                             *
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
#include "MiniShader.h"
#include "MiniRenderer.h"



// function prototypes
void CreateViewport(float w, float h);

//------------------------------------------------------------------------------
MINI_Shader       g_Shader;
GLuint            g_ShaderProgram    = 0;
float*            g_pVertexBuffer    = 0;
unsigned int      g_VertexCount      = 0;
MINI_Index*       g_pIndexes         = 0;
unsigned int      g_IndexCount       = 0;
float*            g_pCylinderVB      = 0;
unsigned int      g_CylinderVBCount  = 0;
float             g_CircleRadius     = 0.1f;
float             g_CylinderRadius   = 0.1f;
float             g_CylinderHeight   = 2.0f;
float             g_Angle            = 0.0f;
unsigned          g_CylFaceCount     = 12;
int               g_SceneInitialized = 0;
MINI_Size         g_View;
MINI_VertexFormat g_VertexFormat;
MINI_Matrix       g_ProjectionMatrix;
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

    miniGetPerspective(&fov, &aspect, &zNear, &zFar, &g_ProjectionMatrix);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(g_ShaderProgram, "mini_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &g_ProjectionMatrix.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void InitScene(int w, int h)
{
    g_View.m_Width  = 0.0f;
    g_View.m_Height = 0.0f;

    // compile, link and use shader
    g_ShaderProgram = miniCompileShaders(miniGetVSColored(), miniGetFSColored());
    glUseProgram(g_ShaderProgram);

    // get shader attributes
    g_Shader.m_VertexSlot = glGetAttribLocation(g_ShaderProgram, "mini_vPosition");
    g_Shader.m_ColorSlot  = glGetAttribLocation(g_ShaderProgram, "mini_vColor");

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // disable culling
    glDisable(GL_CULL_FACE);

    // create the viewport
    CreateViewport(w, h);

    g_VertexFormat.m_UseNormals  = 0;
    g_VertexFormat.m_UseTextures = 0;
    g_VertexFormat.m_UseColors   = 1;

    // generate sphere
    miniCreateSphere(&g_CircleRadius,
                     5,
                     12,
                     0x0000FFFF,
                     &g_VertexFormat,
                     &g_pVertexBuffer,
                     &g_VertexCount,
                     &g_pIndexes,
                     &g_IndexCount);

    // generate cylinder
    miniCreateCylinder(&g_CylinderRadius,
                       &g_CylinderHeight,
                       g_CylFaceCount,
                       0xFF0000FF,
                       &g_VertexFormat,
                       &g_pCylinderVB,
                       &g_CylinderVBCount);

    g_SceneInitialized = 1;
}
//------------------------------------------------------------------------------
void DeleteScene()
{
    g_SceneInitialized = 0;

    // delete buffer index table
    if (g_pIndexes)
    {
        free(g_pIndexes);
        g_pIndexes = 0;
    }

    // delete vertices
    if (g_pVertexBuffer)
    {
        free(g_pVertexBuffer);
        g_pVertexBuffer = 0;
    }

    // delete cylinder vertices
    if (g_pCylinderVB)
    {
        free(g_pCylinderVB);
        g_pCylinderVB = 0;
    }

    // delete shader program
    if (g_ShaderProgram)
        glDeleteProgram(g_ShaderProgram);

    g_ShaderProgram = 0;
}

struct timeval t1, t2;
static float elapsedTime;

//------------------------------------------------------------------------------
void UpdateScene(float elapsedTime2)

{
    // calculate next angle value, limit to 2 * PI
    g_Angle = fmodf(g_Angle + (3.0f * elapsedTime), M_PI * 2.0f);
    
   // timing
   gettimeofday( &t2, NULL );
   elapsedTime = t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6;
   t1 = t2;
}


//------------------------------------------------------------------------------
void DrawScene()
{
    float           angle;
    MINI_Vector3    t;
    MINI_Vector3    axis;
    MINI_Quaternion quatX;
    MINI_Quaternion quatZ;
    MINI_Matrix     translateMatrix;
    MINI_Matrix     rotTransMatrix;
    MINI_Matrix     rotMatrixX;
    MINI_Matrix     rotMatrixZ;
    MINI_Matrix     combMatrix1;
    MINI_Matrix     combMatrix2;
    MINI_Matrix     modelMatrix;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    t.m_X =  0.95f;
    t.m_Y =  0.0f;
    t.m_Z = -4.0f;

    // set the ball translation
    miniGetTranslateMatrix(&t, &translateMatrix);

    t.m_X = 0.2f;
    t.m_Y = 0.0f;
    t.m_Z = 0.0f;

    // set the ball animation translation
    miniGetTranslateMatrix(&t, &rotTransMatrix);

    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;
    angle    = g_Angle;

    // calculate the x axis rotation
    miniQuatFromAxis(&angle, &axis, &quatX);
    miniGetMatrix(&quatX, &rotMatrixX);

    axis.m_X = 0.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 1.0f;
    angle    = M_PI / 2.0f;

    // calculate the z axis rotation
    miniQuatFromAxis(&angle, &axis, &quatZ);
    miniGetMatrix(&quatZ, &rotMatrixZ);

    // build the ball final model matrix
    miniMatrixMultiply(&rotTransMatrix, &rotMatrixZ,      &combMatrix1);
    miniMatrixMultiply(&combMatrix1,    &rotMatrixX,      &combMatrix2);
    miniMatrixMultiply(&combMatrix2,    &translateMatrix, &modelMatrix);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(g_ShaderProgram, "mini_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelMatrix.m_Table[0][0]);

    // draw the sphere
    miniDrawSphere(g_pVertexBuffer,
                   g_VertexCount,
                   g_pIndexes,
                   g_IndexCount,
                   &g_VertexFormat,
                   &g_Shader);

    t.m_X =  0.0f;
    t.m_Y =  0.0f;
    t.m_Z = -4.0f;

    // set the cylinder animation translation
    miniGetTranslateMatrix(&t, &translateMatrix);

    // build the cylinder final model matrix
    miniMatrixMultiply(&rotMatrixZ,  &rotMatrixX,      &combMatrix1);
    miniMatrixMultiply(&combMatrix1, &translateMatrix, &modelMatrix);

    // connect model view matrix to shader
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelMatrix.m_Table[0][0]);

    // draw the cylinder
    miniDrawCylinder(g_pCylinderVB, g_CylFaceCount, &g_VertexFormat, &g_Shader);

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
    wcex.lpszClassName = "MAPI_quaternion";

    if (!RegisterClassEx(&wcex))
        return 0;

    // create the main window
    hWnd = CreateWindowEx(0,
                          "MAPI_quaternion",
                          "MiniAPI Quaternion Demo",
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
