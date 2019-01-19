/*****************************************************************************
 * ==> Ocean water effect ---------------------------------------------------*
 *****************************************************************************
 * Description : Ocean water effect, based on the Gerstner wave algorithm    *
 * Developer   : Jean-Milost Reymond                                         *
 *****************************************************************************/



#ifdef CCR_FORCE_LLVM_INTERPRETER
    #error "Clang/LLVM on iOS does not support function pointer yet. Consider using CPP built-in compiler."
#endif

// std
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// OpenGL
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

//------------------------------------------------------------------------------
typedef struct 
{
    float m_X; // x coordinate for the 3D vector
    float m_Y; // y coordinate for the 3D vector
    float m_Z; // z coordinate for the 3D vector
} QR_Vector3;
//------------------------------------------------------------------------------
typedef struct 
{
    float m_Table[4][4]; // 4x4 matrix array
} QR_Matrix;
//------------------------------------------------------------------------------
typedef struct
{
    int m_Start;
    int m_Length;
} QR_Index;
//------------------------------------------------------------------------------
typedef struct
{
    float m_Position[3];
    float m_Color[4];
} QR_Vertex;
//------------------------------------------------------------------------------
const char* pVertexShader =
    "attribute vec4 qr_aPosition;"
    "attribute vec4 qr_aSourceColor;"
    "varying   vec4 qr_vDestinationColor;"
    "uniform   mat4 qr_uProjection;"
    "uniform   mat4 qr_uModelview;"
    "void main(void)"
    "{"
    "    qr_vDestinationColor = qr_aSourceColor;"
    "    gl_Position          = qr_uProjection * qr_uModelview * qr_aPosition;"
    "}";
//------------------------------------------------------------------------------
const char* pFragmentShader =
    "varying lowp vec4 qr_vDestinationColor;"
    "void main(void)"
    "{"
    "    gl_FragColor = qr_vDestinationColor;"
    "}";
//------------------------------------------------------------------------------
// renderer buffers should no more be generated since CCR version 1.1
#if ((__CCR__ < 1) || ((__CCR__ == 1) && (__CCR_MINOR__ < 1)))
    #ifndef ANDROID
        GLuint g_Renderbuffer, g_Framebuffer;
    #endif
#endif
GLuint     g_ShaderProgram;
QR_Vertex* g_pVertexes     = 0;
int        g_VertexCount   = 0;
QR_Index*  g_pIndexes      = 0;
int        g_IndexCount    = 0;
float      g_TimeElapsed   = 0.0f;
GLuint     g_PositionSlot;
GLuint     g_ColorSlot;
//------------------------------------------------------------------------------
void Dot(QR_Vector3* pV1, QR_Vector3* pV2, float* pR)
{
    *pR = ((pV1->m_X * pV2->m_X) + (pV1->m_Y * pV2->m_Y) + (pV1->m_Z * pV2->m_Z));
}
//------------------------------------------------------------------------------
void WaveGenerator(float* pX, float* pZ, float* pT, QR_Vector3* pR)
{
    float c    = 10.0f; // wave direction constant
    float alt  = 10.0f; // wave altitude - it's a scaling factor
    float l    = 15.0f; // wave length
    float nper = 5.0f;  // wave amplitude - higher = quiet, smaller = agitated
    float wa   = (2.0f * 3.1415f) / l;
    float q    = 1.0f / ((alt * wa) * nper);

    QR_Vector3 pos;
    pos.m_X = *pX;
    pos.m_Y = 0.0f;
    pos.m_Z = *pZ;

    QR_Vector3 d;
    d.m_X =  0.3f;
    d.m_Y =  0.3f;
    d.m_Z = -0.3f;

    float direction;

    // calculate direction
    Dot(&d, &pos, &direction);

    // calculate final vector applying the Gerstner algorithm
    pR->m_X += q * alt * d.m_X * cosf((2.0f * 3.1415f * (direction - c * *pT)) / l);
    pR->m_Y  = q * alt *         sinf((2.0f * 3.1415f * (direction - c * *pT)) / l);
    pR->m_Z += q * alt * d.m_Z * cosf((2.0f * 3.1415f * (direction - c * *pT)) / l);
}
//------------------------------------------------------------------------------
void CreateWaterPlane(int width, int height, float* pT)
{
    // NOTE For demonstration purposes, the waves are calculated outside the
    // vertex shader, in the above and following functions. However, in normal
    // conditions, it should be better to calculate the sine wave effect inside
    // the vertex shader, and not outside, to gain (many) performances

    // delete buffer index table
    if (g_pIndexes)
    {
        free(g_pIndexes);
        g_pIndexes = 0;
    }

    // delete vertexes
    if (g_pVertexes)
    {
        free(g_pVertexes);
        g_pVertexes = 0;
    }

    g_IndexCount  = 0;
    g_VertexCount = 0;

    // calculate triangle edge size and start position
    float triangleSide = 3.0f;
    float startX       = -((width  >> 1) * triangleSide);
    float startZ       = 0.0f;

    for (int j = 0; j < height - 1; ++j)
    {
        // increase index count
        ++g_IndexCount;

        // generate new index in table
        if (!g_pIndexes)
            g_pIndexes = (QR_Index*)malloc(sizeof(QR_Index));
        else
            g_pIndexes = (QR_Index*)realloc(g_pIndexes,
                                            g_IndexCount * sizeof(QR_Index));

        // calculate current index and slice fan length
        int iIndex    = g_IndexCount - 1;
        int fanLength = (width * 2);

        // populate index
        g_pIndexes[iIndex].m_Start  = g_VertexCount;
        g_pIndexes[iIndex].m_Length = fanLength;

        // calculate new vertex buffer length
        g_VertexCount += fanLength;

        // generate vertexes
        if (!g_pVertexes)
            g_pVertexes = (QR_Vertex*)malloc(g_VertexCount * sizeof(QR_Vertex));
        else
            g_pVertexes = (QR_Vertex*)realloc(g_pVertexes,
                                              g_VertexCount * sizeof(QR_Vertex));

        for (int i = 0; i < width; ++i)
        {
            // calculate next index
            int index = g_pIndexes[iIndex].m_Start + (i * 2);

            // calculate initial x and y positions
            float x = startX + (i * triangleSide);
            float y = 0.0f;
            float z;

            // first triangle fan?
            if (!j)
            {
                // calculate initial z position
                z = startZ - (j * triangleSide);

                // build top vertex on plane
                QR_Vector3 topVec;
                topVec.m_X = x;
                topVec.m_Y = y;
                topVec.m_Z = z;

                // update top vertex to follow the wave
                WaveGenerator(&x, &z, pT, &topVec);

                // set top vertex data
                g_pVertexes[index].m_Position[0] = topVec.m_X;
                g_pVertexes[index].m_Position[1] = topVec.m_Y * 10.0f;
                g_pVertexes[index].m_Position[2] = topVec.m_Z;
            }
            else
            {
                // copy top vertex data from previous triangle fan bottom vertex
                g_pVertexes[index].m_Position[0] = g_pVertexes[index + 1 - fanLength].m_Position[0];
                g_pVertexes[index].m_Position[1] = g_pVertexes[index + 1 - fanLength].m_Position[1];
                g_pVertexes[index].m_Position[2] = g_pVertexes[index + 1 - fanLength].m_Position[2];
            }

            // calculate initial z position
            z = startZ - ((j + 1) * triangleSide);

            // build bottom vertex on plane
            QR_Vector3 btmVec;
            btmVec.m_X = x;
            btmVec.m_Y = y;
            btmVec.m_Z = z;

            // update bottom vertex to follow the wave
            WaveGenerator(&x, &z, pT, &btmVec);

             // set bottom vertex data
            g_pVertexes[index + 1].m_Position[0] = btmVec.m_X;
            g_pVertexes[index + 1].m_Position[1] = btmVec.m_Y * 10.0f;
            g_pVertexes[index + 1].m_Position[2] = btmVec.m_Z;

            // set color data (2 tons of blue)
            g_pVertexes[index].m_Color[0]     = j % 2 ? 0.08f : 0.0f;
            g_pVertexes[index].m_Color[1]     = j % 2 ? 0.27f : 0.0f;
            g_pVertexes[index].m_Color[2]     = j % 2 ? 0.98f : 1.0f;
            g_pVertexes[index].m_Color[3]     = 1.0f;
            g_pVertexes[index + 1].m_Color[0] = j % 2 ? 0.0f : 0.08f;
            g_pVertexes[index + 1].m_Color[1] = j % 2 ? 0.0f : 0.27f;
            g_pVertexes[index + 1].m_Color[2] = j % 2 ? 1.0f : 0.98f;
            g_pVertexes[index + 1].m_Color[3] = 1.0f;
        }
    }
}
//------------------------------------------------------------------------------
void GetIdentity(QR_Matrix* pM)
{
    pM->m_Table[0][0] = 1.0f; pM->m_Table[1][0] = 0.0f; pM->m_Table[2][0] = 0.0f; pM->m_Table[3][0] = 0.0f;
    pM->m_Table[0][1] = 0.0f; pM->m_Table[1][1] = 1.0f; pM->m_Table[2][1] = 0.0f; pM->m_Table[3][1] = 0.0f;
    pM->m_Table[0][2] = 0.0f; pM->m_Table[1][2] = 0.0f; pM->m_Table[2][2] = 1.0f; pM->m_Table[3][2] = 0.0f;
    pM->m_Table[0][3] = 0.0f; pM->m_Table[1][3] = 0.0f; pM->m_Table[2][3] = 0.0f; pM->m_Table[3][3] = 1.0f;
}
//------------------------------------------------------------------------------
void GetOrtho(float*     pLeft,
              float*     pRight,
              float*     pBottom,
              float*     pTop,
              float*     pZNear,
              float*     pZFar,
              QR_Matrix* pM)
{
    // calculate matrix component values
    float pfn = *pZFar  + *pZNear;
    float mnf = *pZNear - *pZFar;
    float prl = *pRight + *pLeft;
    float mrl = *pRight - *pLeft;
    float ptb = *pTop   + *pBottom;
    float mtb = *pTop   - *pBottom;
    float mlr = -mrl;
    float mbt = -mtb;

    // build matrix
    pM->m_Table[0][0] = 2.0f / mrl; pM->m_Table[1][0] = 0.0f;       pM->m_Table[2][0] = 0.0f;       pM->m_Table[3][0] = prl / mlr;
    pM->m_Table[0][1] = 0.0f;       pM->m_Table[1][1] = 2.0f / mtb; pM->m_Table[2][1] = 0.0f;       pM->m_Table[3][1] = ptb / mbt;
    pM->m_Table[0][2] = 0.0f;       pM->m_Table[1][2] = 0.0f;       pM->m_Table[2][2] = 2.0f / mnf; pM->m_Table[3][2] = pfn / mnf;
    pM->m_Table[0][3] = 0.0f;       pM->m_Table[1][3] = 0.0f;       pM->m_Table[2][3] = 0.0f;       pM->m_Table[3][3] = 1.0f;
}
//------------------------------------------------------------------------------
void GetRotateMatrix(float* pAngle, QR_Vector3* pAxis, QR_Matrix* pM)
{
    // calculate sinus, cosinus and inverted cosinus values
    float c  = cosf(*pAngle);
    float s  = sinf(*pAngle);
    float ic = (1.0f - c);

    // create rotation matrix
    GetIdentity(pM);
    pM->m_Table[0][0] = (ic * pAxis->m_X * pAxis->m_X) + c;
    pM->m_Table[1][0] = (ic * pAxis->m_X * pAxis->m_Y) - (s * pAxis->m_Z);
    pM->m_Table[2][0] = (ic * pAxis->m_X * pAxis->m_Z) + (s * pAxis->m_Y);
    pM->m_Table[0][1] = (ic * pAxis->m_Y * pAxis->m_X) + (s * pAxis->m_Z);
    pM->m_Table[1][1] = (ic * pAxis->m_Y * pAxis->m_Y) + c;
    pM->m_Table[2][1] = (ic * pAxis->m_Y * pAxis->m_Z) - (s * pAxis->m_X);
    pM->m_Table[0][2] = (ic * pAxis->m_Z * pAxis->m_X) - (s * pAxis->m_Y);
    pM->m_Table[1][2] = (ic * pAxis->m_Z * pAxis->m_Y) + (s * pAxis->m_X);
    pM->m_Table[2][2] = (ic * pAxis->m_Z * pAxis->m_Z) + c;
}
//------------------------------------------------------------------------------
GLuint CreateAndCompileShader(const char* pSource, GLenum shaderType)
{
    // create and compile shader program
    GLuint shaderHandle = glCreateShader(shaderType);
    glShaderSource(shaderHandle, 1, &pSource, 0);
    glCompileShader(shaderHandle);

    // get compiler result
    GLint compileSuccess;
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);

    // succeeded?
    if (compileSuccess == GL_FALSE)
    {
        // show error message (in console)
        GLchar messages[256];
        glGetShaderInfoLog(shaderHandle, sizeof(messages), 0, &messages[0]);
        printf("compile glsl error : %s\n", messages);
    }

    return shaderHandle;
}
//------------------------------------------------------------------------------
GLuint CompileShaders(const char* pVShader,const char* pFShader)
{
    // create and compile vertex and fragment shaders programs
    GLuint vertexShader   = CreateAndCompileShader(pVShader, GL_VERTEX_SHADER);
    GLuint fragmentShader = CreateAndCompileShader(pFShader, GL_FRAGMENT_SHADER);

    // link shader programs
    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vertexShader);
    glAttachShader(programHandle, fragmentShader);
    glLinkProgram(programHandle);

    // get linker result
    GLint linkSuccess;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);

    // succeeded?
    if (linkSuccess == GL_FALSE)
    {
        // show error message (in console)
        GLchar messages[256];
        glGetProgramInfoLog(programHandle, sizeof(messages), 0, &messages[0]);
        printf("compile glsl error : %s\n", messages);
    }

    return programHandle;
}
//------------------------------------------------------------------------------
void ApplyOrtho(float maxX, float maxY) 
{
    // get orthogonal matrix
    float     left   = -12.0f;
    float     right  =  12.0f;
    float     bottom = -18.0f;
    float     top    =  18.0f;
    float     near   =  0.1f;
    float     far    =  100.0f;
    QR_Matrix ortho;
    GetOrtho(&left, &right, &bottom, &top, &near, &far, &ortho);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(g_ShaderProgram, "qr_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &ortho.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void on_GLES2_Size(int view_w, int view_h)
{
    glViewport(0, 0, view_w, view_h);
    ApplyOrtho(2.0f, 3.0f);
}
//------------------------------------------------------------------------------
int on_GLES2_Init(int view_w, int view_h)
{
    // compile, link and use shaders
    g_ShaderProgram = CompileShaders(pVertexShader, pFragmentShader);
    glUseProgram(g_ShaderProgram);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // set rotation axis
    QR_Vector3 r;
    r.m_X = 1.0f;
    r.m_Y = 0.0f;
    r.m_Z = 0.0f;

    // set rotation angle
    float angle = -0.5f;

    // calculate model view matrix (it's a rotation on the y axis)
    QR_Matrix modelViewMatrix;
    GetRotateMatrix(&angle, &r, &modelViewMatrix);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(g_ShaderProgram, "qr_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);
    
    // set viewport
    on_GLES2_Size(view_w, view_h);

    return 1;
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    // delete buffer index table
    if (g_pIndexes)
    {
        free(g_pIndexes);
        g_pIndexes = 0;
    }

    // delete vertexes
    if (g_pVertexes)
    {
        free(g_pVertexes);
        g_pVertexes = 0;
    }

    // delete shader program
    if (g_ShaderProgram)
        glDeleteProgram(g_ShaderProgram);

    g_ShaderProgram = 0;
}

//------------------------------------------------------------------------------
void on_GLES2_Update(float timeStep_sec)
{
    // calculate elapsed time
    g_TimeElapsed += 0.02f;//timeStep_sec;

    // do reduce elapsed time? (to make animation cycle infinite)
    if (g_TimeElapsed >= 100000.0f)
        g_TimeElapsed -= 100000.0f;

    // generate water plane
    CreateWaterPlane(20, 20, &g_TimeElapsed);
}
//------------------------------------------------------------------------------
void on_GLES2_Render()
{
    // clear scene background and depth buffer
    glClearColor(0.55f, 0.96f, 0.96f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	
    // get vertex and color slots
    g_PositionSlot = glGetAttribLocation(g_ShaderProgram, "qr_aPosition");
    g_ColorSlot    = glGetAttribLocation(g_ShaderProgram, "qr_aSourceColor");

    // calculate vertex stride
    int stride = sizeof(QR_Vertex);

    // enable position and color slots
    glEnableVertexAttribArray(g_PositionSlot);
    glEnableVertexAttribArray(g_ColorSlot);

    // iterate through vertex fan buffers to draw
    for (int i = 0; i < g_IndexCount; ++i)
    {
        // get next vertexes fan buffer
        GLvoid* pCoords = &g_pVertexes[g_pIndexes[i].m_Start].m_Position[0];
        GLvoid* pColors = &g_pVertexes[g_pIndexes[i].m_Start].m_Color[0];

        // connect buffer to shader
        glVertexAttribPointer(g_PositionSlot, 3, GL_FLOAT, GL_FALSE, stride, pCoords);
        glVertexAttribPointer(g_ColorSlot,    4, GL_FLOAT, GL_FALSE, stride, pColors);

        // draw it
        glDrawArrays(GL_TRIANGLE_STRIP, 0, g_pIndexes[i].m_Length);
    }

    // disconnect slots from shader
    glDisableVertexAttribArray(g_PositionSlot);
    glDisableVertexAttribArray(g_ColorSlot);
}
//------------------------------------------------------------------------------
void on_GLES2_TouchBegin(float x, float y)
{}
//------------------------------------------------------------------------------
void on_GLES2_TouchEnd(float x, float y)
{}
//------------------------------------------------------------------------------
void on_GLES2_TouchMove(float prev_x, float prev_y, float x, float y)
{}
//------------------------------------------------------------------------------
