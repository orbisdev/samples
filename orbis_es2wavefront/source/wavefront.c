/*****************************************************************************
 * ==> WaveFront (.obj) file reader -----------------------------------------*
 *****************************************************************************
 * Description : Simple WaveFront (.obj) file reader                         *
 * Developer   : Jean-Milost Reymond                                         *
 *****************************************************************************/

// supported platforms check. NOTE iOS, Android and Windows only, but may works on other platforms ;-)

#ifdef CCR_FORCE_LLVM_INTERPRETER
    #error "Clang/LLVM on iOS does not support function pointer yet. Consider using CPP built-in compiler."
#endif

// std
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// OpenGL
#include <debugnet.h>
#include <orbisGl.h>

//------------------------------------------------------------------------------
#include "Model.obj.h"
/*
    include Model object from header,
    this give us buffer and lenght as:
    unsigned char Model_obj[];
    unsigned int  Model_obj_len;
*/

//------------------------------------------------------------------------------
struct QR_Vector3
{
    float m_X; // x coordinate for the 3D vector
    float m_Y; // y coordinate for the 3D vector
    float m_Z; // z coordinate for the 3D vector
};
//------------------------------------------------------------------------------
struct QR_Matrix
{
    float m_Table[4][4]; // 4x4 matrix array
};
//------------------------------------------------------------------------------
struct QR_Vertex
{
    float m_Position[3];
    float m_Color[4];
};
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
GLuint     g_ShaderProgram;
struct QR_Vertex* g_pVertexes     = 0;
int        g_VertIndex     = 0;
int        g_VertCount     = 0;
float      g_RotationSpeed = 0.02f;
float      g_Angle         = 0.0f;
GLuint     g_PositionSlot;
GLuint     g_ColorSlot;
//------------------------------------------------------------------------------
void ParseWaveFrontFile(void)
{
    char   buffer[256];
    float* pVectors   = 0;
    int*   pFaces     = 0;
    int    index      = 0;
    int    vecCount   = 0;
    int    faceCount  = 0;
    int    readVector = 0;
    int    readFace   = 0;

    // open wavefront data
    int ch = 0;

    // iterate through data chars
    for(int i=0; i<Model_obj_len; i++)
    {
        ch = Model_obj[i];

        // dispatch char
        switch (ch)
        {
            // found commented line
            case '#':
                printf("COMMENT - ");

                // skip line, but log it
                while (ch != '\r' && ch != '\n' && ch != EOF)
                {
                    // log next char
                    printf("%c", ch);

                    // go to next char
                    ch = Model_obj[++i];
                }

                printf("\n");
                continue;

            // found separator
            case '\r':
            case '\n':
            case ' ':
            case '/':
                // previously read buffer contains numeric value to interpret,
                // or is a vector or a face command?
                if ((buffer[0] >= '0' && buffer[0] <= '9') || buffer[0] == '-')
                {
                    // is numeric value part of a vector or a face?
                    if (readVector)
                    {
                        // value to read is part of a vector
                        ++vecCount;

                        // allocate memory for vector numeric value
                        if (pVectors)
                            pVectors = (float*)realloc(pVectors, vecCount * sizeof(float));
                        else
                            pVectors = (float*)malloc(sizeof(float));

                        // set line end indicator in buffer to convert
                        buffer[index] = '\0';

                        // convert read value to float and add it to current vector
                        pVectors[vecCount - 1] = atof(buffer);
                    }
                    else
                    if (readFace)
                    {
                        // value to read is part of a face, check if value is a
                        // vector index, or if final vertexes should be calculated
                        if (ch == '/')
                        {
                            // read value is part of a face
                            ++faceCount;

                            // allocate memory for face index
                            if (pFaces)
                                pFaces = (int*)realloc(pFaces, faceCount * sizeof(int));
                            else
                                pFaces = (int*)malloc(sizeof(int));

                            // set line end indicator in buffer to convert
                            buffer[index] = '\0';

                            // convert read value to float
                            pFaces[faceCount - 1] = atoi(buffer);
                        }
                        else
                        if (ch == '\r' || ch == '\n')
                        {
                            // final vertexes should be calculated from face
                            // index list. Calculate new vertexes count
                            g_VertCount += 3 * (faceCount - 2);

                            // allocate memory for new vertexes
                            if (!g_pVertexes)
                                g_pVertexes =
                                        (struct QR_Vertex*)malloc(g_VertCount * sizeof(struct QR_Vertex));
                            else
                                g_pVertexes =
                                        (struct QR_Vertex*)realloc(g_pVertexes,
                                                            g_VertCount * sizeof(struct QR_Vertex));

                            // wavefront faces are organized as triangle fan, so
                            // get the first vertex and build all others from it
                            int baseVecIndex = (pFaces[0] - 1) * 3;

                            // iterate through remaining indexes
                            for (int i = 1; i <= faceCount - 2; ++i)
                            {
                                // build polygon vertex 1
                                int vecIndex = baseVecIndex;

                                g_pVertexes[g_VertIndex].m_Position[0] = pVectors[vecIndex];
                                g_pVertexes[g_VertIndex].m_Position[1] = pVectors[vecIndex + 1];
                                g_pVertexes[g_VertIndex].m_Position[2] = pVectors[vecIndex + 2];

                                g_pVertexes[g_VertIndex].m_Color[0] = 1.0f;
                                g_pVertexes[g_VertIndex].m_Color[1] = 1.0f;
                                g_pVertexes[g_VertIndex].m_Color[2] = 1.0f;
                                g_pVertexes[g_VertIndex].m_Color[3] = 1.0f;

                                // build polygon vertex 2
                                vecIndex = (pFaces[i] - 1) * 3;

                                g_pVertexes[g_VertIndex + 1].m_Position[0] = pVectors[vecIndex];
                                g_pVertexes[g_VertIndex + 1].m_Position[1] = pVectors[vecIndex + 1];
                                g_pVertexes[g_VertIndex + 1].m_Position[2] = pVectors[vecIndex + 2];

                                g_pVertexes[g_VertIndex + 1].m_Color[0] = 1.0f;
                                g_pVertexes[g_VertIndex + 1].m_Color[1] = 1.0f;
                                g_pVertexes[g_VertIndex + 1].m_Color[2] = 1.0f;
                                g_pVertexes[g_VertIndex + 1].m_Color[3] = 1.0f;

                                // build polygon vertex 3
                                vecIndex = (pFaces[i + 1] - 1) * 3;

                                g_pVertexes[g_VertIndex + 2].m_Position[0] = pVectors[vecIndex];
                                g_pVertexes[g_VertIndex + 2].m_Position[1] = pVectors[vecIndex + 1];
                                g_pVertexes[g_VertIndex + 2].m_Position[2] = pVectors[vecIndex + 2];

                                g_pVertexes[g_VertIndex + 2].m_Color[0] = 1.0f;
                                g_pVertexes[g_VertIndex + 2].m_Color[1] = 1.0f;
                                g_pVertexes[g_VertIndex + 2].m_Color[2] = 1.0f;
                                g_pVertexes[g_VertIndex + 2].m_Color[3] = 1.0f;

                                // go to next polygon
                                g_VertIndex += 3;
                            }

                            // delete face index buffer
                            free(pFaces);
                            pFaces    = 0;
                            faceCount = 0;
                        }
                    }
                }
                else
                if (buffer[0] == 'v' && index == 1)
                {
                    // needs to read a vector
                    readVector = 1;
                    readFace   = 0;
                }
                else
                if (buffer[0] == 'f' && index == 1)
                {
                    // needs to read a face
                    readVector = 0;
                    readFace   = 1;
                }
                else
                {
                    // other command, not exploited here
                    readVector = 0;
                    readFace   = 0;
                }

                // reset value buffer
                buffer[0] = '\0';
                index     = 0;
                break;

            default:
            {
                // copy next read char to value buffer
                buffer[index] = (char)ch;
                ++index;
                break;
            }
        }
    }

    // delete vector table
    free(pVectors);

    // log file result
    debugNetPrintf(INFO,"SUCCESS - parsed %i vertexes\n", g_VertCount);
}
//------------------------------------------------------------------------------
void GetIdentity(struct QR_Matrix* pM)
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
              struct QR_Matrix* pM)
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
void GetTranslateMatrix(struct QR_Vector3* pT, struct QR_Matrix* pM)
{
    GetIdentity(pM);
    pM->m_Table[3][0] += pM->m_Table[0][0] * pT->m_X + pM->m_Table[1][0] * pT->m_Y + pM->m_Table[2][0] * pT->m_Z;
    pM->m_Table[3][1] += pM->m_Table[0][1] * pT->m_X + pM->m_Table[1][1] * pT->m_Y + pM->m_Table[2][1] * pT->m_Z;
    pM->m_Table[3][2] += pM->m_Table[0][2] * pT->m_X + pM->m_Table[1][2] * pT->m_Y + pM->m_Table[2][2] * pT->m_Z;
    pM->m_Table[3][3] += pM->m_Table[0][3] * pT->m_X + pM->m_Table[1][3] * pT->m_Y + pM->m_Table[2][3] * pT->m_Z;
}
//------------------------------------------------------------------------------
void GetRotateMatrix(float* pAngle, struct QR_Vector3* pAxis, struct QR_Matrix* pM)
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
void MatrixMultiply(struct QR_Matrix* pM1, struct QR_Matrix* pM2, struct QR_Matrix* pR)
{
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            pR->m_Table[i][j] = pM1->m_Table[i][0] * pM2->m_Table[0][j] +
                                pM1->m_Table[i][1] * pM2->m_Table[1][j] +
                                pM1->m_Table[i][2] * pM2->m_Table[2][j] +
                                pM1->m_Table[i][3] * pM2->m_Table[3][j];
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
        debugNetPrintf(ERROR,"compile glsl error : %s\n", messages);
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
        debugNetPrintf(ERROR,"compile glsl error : %s\n", messages);
    }

    return programHandle;
}
//------------------------------------------------------------------------------
void ApplyOrtho(float maxX, float maxY, int vertical)
{
    // get orthogonal matrix
    float     left;
    float     right;
    float     bottom;
    float     top;
    float     near   =  1.0f;
    float     far    =  20.0f;

    if (vertical)
    {
        left   = -12.0f;
        right  =  12.0f;
        bottom = -18.0f;
        top    =  18.0f;
    }
    else
    {
        left   = -18.0f;
        right  =  18.0f;
        bottom = -12.0f;
        top    =  12.0f;
    }

    struct QR_Matrix ortho;
    GetOrtho(&left, &right, &bottom, &top, &near, &far, &ortho);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(g_ShaderProgram, "qr_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &ortho.m_Table[0][0]);
}

//------------------------------------------------------------------------------
void on_GLES2_Size(int view_w, int view_h)
{
    int orientation;

    if (view_w < view_h)
        orientation = 1;
    else
        orientation = 0;

    glViewport(0, 0, view_w, view_h);
    ApplyOrtho(2.0f, 3.0f, orientation);
}

//------------------------------------------------------------------------------
void on_GLES2_Init(int view_w, int view_h)
{
    // compile, link and use shaders
    g_ShaderProgram = CompileShaders(pVertexShader, pFragmentShader);
    glUseProgram(g_ShaderProgram);

    ParseWaveFrontFile();

    g_PositionSlot = glGetAttribLocation(g_ShaderProgram, "qr_aPosition");
    g_ColorSlot    = glGetAttribLocation(g_ShaderProgram, "qr_aSourceColor");

    GLvoid* pCoords = &g_pVertexes[0].m_Position[0];
    GLvoid* pColors = &g_pVertexes[0].m_Color[0];

     // calculate vertex stride
    GLsizei stride = sizeof(struct QR_Vertex);

   // connect object to shader
    glVertexAttribPointer(g_PositionSlot, 3, GL_FLOAT, GL_FALSE, stride, pCoords);
    glVertexAttribPointer(g_ColorSlot,    4, GL_FLOAT, GL_FALSE, stride, pColors);

    // set viewport
    on_GLES2_Size(view_w, view_h);
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
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
    g_Angle += g_RotationSpeed;

    if (g_Angle >= 6.28f)
        g_Angle -= 6.28f;
}
//------------------------------------------------------------------------------
void on_GLES2_Render()
{
    // clear scene background and depth buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // enable position and color slots
    glEnableVertexAttribArray(g_PositionSlot);
    glEnableVertexAttribArray(g_ColorSlot);

    // set translation
    struct QR_Vector3 t;
    t.m_X =  0.0f;
    t.m_Y = -6.0f;
    t.m_Z = -10.0f+ 2 * g_Angle;

    struct QR_Matrix modelViewMatrix;
    GetTranslateMatrix(&t, &modelViewMatrix);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(g_ShaderProgram, "qr_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrix.m_Table[0][0]);

    // draw it
    glDrawArrays(GL_TRIANGLES, 0, g_VertCount);

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
