/*****************************************************************************
 * ==> Rover demo -----------------------------------------------------------*
 *****************************************************************************
 * Description : A simple rover model, press the left or right arrow keys to *
 *               increase or decrease the rotation speed                     *
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
#include "MiniModels.h"
#include "MiniShader.h"
#include "MiniRenderer.h"



// function prototypes
void CreateViewport(float w, float h);

//------------------------------------------------------------------------------
MINI_Shader       g_Shader;
GLuint            g_ShaderProgram    = 0;
float*            g_pVertices        = 0;
unsigned          g_VertexCount      = 0;
MINI_MdlCmds*     g_pMdlCmds         = 0;
MINI_Index*       g_pIndexes         = 0;
unsigned          g_IndexCount       = 0;
float             g_Angle            = 0.0f;
float             g_RotationSpeed    = 0.2f;
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

        case WM_KEYDOWN:
            switch (wParam)
            {
                case VK_LEFT:  g_RotationSpeed -= 0.005; break;
                case VK_RIGHT: g_RotationSpeed += 0.005; break;
            }

            break;

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
    const float zFar   = 100.0f;
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
    g_ShaderProgram = miniCompileShaders(miniGetVSColored(), miniGetFSColored());
    glUseProgram(g_ShaderProgram);

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

    // create the rover
    miniCreateRover(&g_VertexFormat,
                    &g_pVertices,
                    &g_VertexCount,
                    &g_pMdlCmds,
                    &g_pIndexes,
                    &g_IndexCount);

    g_SceneInitialized = 1;
}
//------------------------------------------------------------------------------
void DeleteScene()
{
    g_SceneInitialized = 0;

    // delete model commands
    if (g_pMdlCmds)
    {
        free(g_pMdlCmds);
        g_pMdlCmds = 0;
    }

    // delete buffer index table
    if (g_pIndexes)
    {
        free(g_pIndexes);
        g_pIndexes = 0;
    }

    // delete vertices
    if (g_pVertices)
    {
        free(g_pVertices);
        g_pVertices = 0;
    }

    // delete shader program
    if (g_ShaderProgram)
    {
        glDeleteProgram(g_ShaderProgram);
        g_ShaderProgram = 0;
    }
}

struct timeval t1, t2;
static float elapsedTime;

//------------------------------------------------------------------------------
void UpdateScene(float elapsedTime2)
{
	// timing
	gettimeofday( &t2, NULL );
	elapsedTime = t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6;
	t1 = t2;
	
    // calculate next rotation angle
    g_Angle += (g_RotationSpeed * elapsedTime * 0.2f);

    // is rotating angle out of bounds?
    while (g_Angle >= 6.28f)
        g_Angle -= 6.28f;
	

}
//------------------------------------------------------------------------------
void DrawScene()
{
	float        angle;
    MINI_Vector3 t;
    MINI_Vector3 axis;
    MINI_Vector3 factor;
    MINI_Matrix  translateMatrix;
    MINI_Matrix  rotateMatrixX;
    MINI_Matrix  rotateMatrixY;
    MINI_Matrix  scaleMatrix;
    MINI_Matrix  modelViewMatrix;
    float        angleX;
    float        angleY;
    GLint        modelviewUniform;

    miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);

    // set translation
    t.m_X =   0.0f;
    t.m_Y =   0.0f;
    t.m_Z = -10.0f;

    miniGetTranslateMatrix(&t, &translateMatrix);

    // set rotation axis
    axis.m_X = 1.0f;
    axis.m_Y = 0.0f;
    axis.m_Z = 0.0f;

    // set rotation angle
    angleX = 0.0f;

    miniGetRotateMatrix(&angleX, &axis, &rotateMatrixX);

    // set rotation axis
    axis.m_X = 0.0f;
    axis.m_Y = 1.0f;
    axis.m_Z = 0.0f;

    angleY = g_Angle;
	angle    = angleY;
	angleY = M_PI / 2.0f;
	angle    = M_PI / 2.0f;
	
	miniGetRotateMatrix(&angle, &axis, &rotateMatrixY);
	
    //miniGetRotateMatrix(&angleY, &axis, &rotateMatrixY);

    // set scale factor
    factor.m_X = 1.0f;
    factor.m_Y = 1.0f;
    factor.m_Z = 1.0f;

    miniGetScaleMatrix(&factor, &scaleMatrix);

    // calculate model view matrix
    miniMatrixMultiply(&rotateMatrixY,   &rotateMatrixX,   &modelViewMatrix);
    miniMatrixMultiply(&modelViewMatrix, &translateMatrix, &modelViewMatrix);
    miniMatrixMultiply(&modelViewMatrix, &scaleMatrix,     &modelViewMatrix);

    // connect model view matrix to shader
    modelviewUniform = glGetUniformLocation(g_ShaderProgram, "mini_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // draw the rover model
    miniDrawRover(g_pVertices,
                  g_VertexCount,
                  g_pMdlCmds,
                  g_pIndexes,
                  g_IndexCount,
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
         g_RotationSpeed += 0.05;
         break;
      case 3: //_KEY_DOWN:
         g_RotationSpeed -= 0.05;
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
    wcex.lpszClassName = "MAPI_rover";

    if (!RegisterClassEx(&wcex))
        return 0;

    // create the main window
    hWnd = CreateWindowEx(0,
                          "MAPI_rover",
                          "MiniAPI Rover Demo",
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
