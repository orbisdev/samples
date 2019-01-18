/*****************************************************************************
 * ==> Collision detection --------------------------------------------------*
 *****************************************************************************
 * Description : Simple collision detection demo.                            *
 * Developer   : Jean-Milost Reymond                                         *
 *****************************************************************************/

#ifdef CCR_FORCE_LLVM_INTERPRETER
    #error "Clang/LLVM on iOS does not support function pointer yet. Consider using CPP built-in compiler."
#endif

// std
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

// OpenGL
#include  <GLES2/gl2.h>
#include  <EGL/egl.h>

//------------------------------------------------------------------------------
struct QR_Vector3
{
    float m_X; // x coordinate for the 3D vector
    float m_Y; // y coordinate for the 3D vector
    float m_Z; // z coordinate for the 3D vector
};
//------------------------------------------------------------------------------
struct QR_Plane
{
    float m_A;
    float m_B;
    float m_C;
    float m_D;
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
const struct QR_Vertex firstPolygon_Red[3] =
{
    {{-0.7f, -0.10f, 0.0f}, {1.0f, 0.0f, 0.2f, 1.0f}},
    {{ 0.6f, -0.7f,  0.0f}, {1.0f, 0.4f, 0.0f, 1.0f}},
    {{-0.7f,  0.6f,  0.0f}, {1.0f, 0.2f, 0.4f, 1.0f}},
};
//------------------------------------------------------------------------------
const struct QR_Vertex firstPolygon_Blue[3] =
{
    {{-0.7f, -0.10f, 0.0f}, {0.2f, 0.0f, 0.9f, 1.0f}},
    {{ 0.6f, -0.7f,  0.0f}, {0.0f, 0.4f, 0.8f, 1.0f}},
    {{-0.7f,  0.6f,  0.0f}, {0.2f, 0.4f, 0.7f, 1.0f}},
};
//------------------------------------------------------------------------------
const struct QR_Vertex secondPolygon_Red[3] =
{
    {{-0.1f,  0.11f, 0.0f}, {1.0f, 1.0f, 0.2f, 1.0f}},
    {{-0.8f, -0.2f,  0.0f}, {1.0f, 0.8f, 0.0f, 1.0f}},
    {{ 0.11f, 0.10f, 0.0f}, {1.0f, 0.8f, 0.2f, 1.0f}},
};
//------------------------------------------------------------------------------
const struct QR_Vertex secondPolygon_Blue[3] =
{
    {{-0.1f,  0.11f, 0.0f}, {0.2f, 1.0f, 0.8f, 1.0f}},
    {{-0.8f, -0.2f,  0.0f}, {0.0f, 0.8f, 0.8f, 1.0f}},
    {{ 0.11f, 0.10f, 0.0f}, {0.2f, 1.0f, 0.8f, 1.0f}},
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
// renderer buffers should no more be generated since CCR version 1.1
#if ((__CCR__ < 1) || ((__CCR__ == 1) && (__CCR_MINOR__ < 1)))
    #ifndef ANDROID
        GLuint g_Renderbuffer, g_Framebuffer;
    #endif
#endif
GLuint g_ShaderProgram;
float  g_RotationSpeed = 0.02f;
float  g_Angle         = 0.0f;
//------------------------------------------------------------------------------
void LogVector(struct QR_Vector3* pV)
{
    printf("vector.m_X = %f, vector.m_Y = %f, vector.m_Z = %f\n", pV->m_X, pV->m_Y, pV->m_Z);
}
//------------------------------------------------------------------------------
void LogPlane(struct QR_Plane* pP)
{
    printf("plane.m_A = %f, plane.m_B = %f, plane.m_C = %f, plane.m_D = %f\n", pP->m_A,
                                                                               pP->m_B,
                                                                               pP->m_C,
                                                                               pP->m_D);
}
//------------------------------------------------------------------------------
void LogMatrix(struct QR_Matrix* pM)
{
    printf("matrix[0][0] = %f, matrix[1][0] = %f, matrix[2][0] = %f, matrix[3][0] = %f\n",
            pM->m_Table[0][0], pM->m_Table[1][0], pM->m_Table[2][0], pM->m_Table[3][0]);
    printf("matrix[0][1] = %f, matrix[1][1] = %f, matrix[2][1] = %f, matrix[3][1] = %f\n",
            pM->m_Table[0][1], pM->m_Table[1][1], pM->m_Table[2][1], pM->m_Table[3][1]);
    printf("matrix[0][2] = %f, matrix[1][2] = %f, matrix[2][2] = %f, matrix[3][2] = %f\n",
            pM->m_Table[0][2], pM->m_Table[1][2], pM->m_Table[2][2], pM->m_Table[3][2]);
    printf("matrix[0][3] = %f, matrix[1][3] = %f, matrix[2][3] = %f, matrix[3][3] = %f\n",
            pM->m_Table[0][3], pM->m_Table[1][3], pM->m_Table[2][3], pM->m_Table[3][3]);
}
//------------------------------------------------------------------------------
void Copy(struct QR_Vector3* pS, struct QR_Vector3* pD)
{
    pD->m_X = pS->m_X;
    pD->m_Y = pS->m_Y;
    pD->m_Z = pS->m_Z;
}
//------------------------------------------------------------------------------
void Sub(struct QR_Vector3* pV1, struct QR_Vector3* pV2, struct QR_Vector3* pR)
{
    pR->m_X = pV1->m_X - pV2->m_X;
    pR->m_Y = pV1->m_Y - pV2->m_Y;
    pR->m_Z = pV1->m_Z - pV2->m_Z;
}
//------------------------------------------------------------------------------
void Min(float* pA, float* pB, float* pR)
{
    if (*pA < *pB)
    {
        *pR = *pA;
        return;
    }

    *pR = *pB;
}
//------------------------------------------------------------------------------
void Max(float* pA, float* pB, float* pR)
{
    if (*pA > *pB)
    {
        *pR = *pA;
        return;
    }

    *pR = *pB;
}
//------------------------------------------------------------------------------
void Length(struct QR_Vector3* pV, float* pR)
{
    *pR = sqrt((pV->m_X * pV->m_X) + (pV->m_Y * pV->m_Y) + (pV->m_Z * pV->m_Z));
}
//------------------------------------------------------------------------------
void Normalize(struct QR_Vector3* pV, struct QR_Vector3* pR)
{
    float len;
    Length(pV, &len);

    if (len == 0.0f)
    {
        pR->m_X = 0.0f;
        pR->m_Y = 0.0f;
        pR->m_Z = 0.0f;
        return;
    }

    pR->m_X = (pV->m_X / len);
    pR->m_Y = (pV->m_Y / len);
    pR->m_Z = (pV->m_Z / len);
}
//------------------------------------------------------------------------------
void Cross(struct QR_Vector3* pV1, struct QR_Vector3* pV2, struct QR_Vector3* pR)
{
    pR->m_X = (pV1->m_Y * pV2->m_Z) - (pV2->m_Y * pV1->m_Z);
    pR->m_Y = (pV1->m_Z * pV2->m_X) - (pV2->m_Z * pV1->m_X);
    pR->m_Z = (pV1->m_X * pV2->m_Y) - (pV2->m_X * pV1->m_Y);
}
//------------------------------------------------------------------------------
void Dot(struct QR_Vector3* pV1, struct QR_Vector3* pV2, float* pR)
{
    *pR = ((pV1->m_X * pV2->m_X) + (pV1->m_Y * pV2->m_Y) + (pV1->m_Z * pV2->m_Z));
}
//------------------------------------------------------------------------------
void PlaneFromPointNormal(struct QR_Vector3* pP, struct QR_Vector3* pN, struct QR_Plane* pR)
{
    // the a, b, and c components are only the normal of the plane, and the D
    // component can be calculated using the aX + bY + cZ + d = 0 algorithm
    pR->m_A = pN->m_X;
    pR->m_B = pN->m_Y;
    pR->m_C = pN->m_Z;

    float d;
    Dot(pN, pP, &d);
    pR->m_D = -d;
}
//------------------------------------------------------------------------------
void PlaneFromPoints(struct QR_Vector3* pV1, struct QR_Vector3* pV2, struct QR_Vector3* pV3, struct QR_Plane* pR)
{
    // calculate edge vectors
    struct QR_Vector3 e1;
    Sub(pV2, pV1, &e1);

    struct QR_Vector3 e2;
    Sub(pV3, pV1, &e2);

    // calculate the normal of the plane
    struct QR_Vector3 normal;
    Cross(&e1, &e2, &normal);
    Normalize(&normal, &normal);

    // calculate and return the plane
    PlaneFromPointNormal(pV1, &normal, pR);
}
//------------------------------------------------------------------------------
void DistanceToPlane(struct QR_Vector3* pP, struct QR_Plane* pPl, float* pR)
{
    // get the normal of the plane
    struct QR_Vector3 n;
    n.m_X = pPl->m_A;
    n.m_Y = pPl->m_B;
    n.m_Z = pPl->m_C;

    float dist;

    // calculate the distance between the plane and the point
    Dot(&n, pP, &dist);
    *pR = dist + pPl->m_D;
}
//------------------------------------------------------------------------------
int PlaneIntersectLine(struct QR_Plane* pPl, struct QR_Vector3* pV1, struct QR_Vector3* pV2, struct QR_Vector3* pP)
{
    // gets the normal of the plane
    struct QR_Vector3 n;
    n.m_X = pPl->m_A;
    n.m_Y = pPl->m_B;
    n.m_Z = pPl->m_C;

    // calculates the direction of the line
    struct QR_Vector3 dir;
    Sub(pV2, pV1, &dir);

    // calculates the angle between the line and the normal to the plane
    float dot;
    Dot(&n, &dir, &dot);

    // if normal to the plane is perpendicular to the line, then the line is
    // either parallel to the plane and there are no solutions or the line is
    // on the plane in which case there are an infinite number of solutions
    if (!dot)
        return 0;

    float nDot;
    Dot(&n, pV1, &nDot);

    float temp = ((pPl->m_D + nDot) / dot);

    // calculates the intersection point
    pP->m_X = (pV1->m_X - (temp * dir.m_X));
    pP->m_Y = (pV1->m_Y - (temp * dir.m_Y));
    pP->m_Z = (pV1->m_Z - (temp * dir.m_Z));

    return 1;
}
//------------------------------------------------------------------------------
int ValueIsBetween(float* pV, float* pS, float* pE, float* pEpsylon)
{
    float minVal;
    Min(pS, pE, &minVal);

    float maxVal;
    Max(pS, pE, &maxVal);

    // check if each value is between start and end limits considering tolerance
    if (*pV >= (minVal - *pEpsylon) && *pV <= (maxVal + *pEpsylon))
        return 1;

    return 0;
}
//------------------------------------------------------------------------------
int VectorIsBetween(struct QR_Vector3* pP, struct QR_Vector3* pS, struct QR_Vector3* pE, float* pEpsylon)
{
    // check if each vector component is between start and end limits
    if (!ValueIsBetween(&pP->m_X, &pS->m_X, &pE->m_X, pEpsylon))
        return 0;

    if (!ValueIsBetween(&pP->m_Y, &pS->m_Y, &pE->m_Y, pEpsylon))
        return 0;

    if (!ValueIsBetween(&pP->m_Z, &pS->m_Z, &pE->m_Z, pEpsylon))
        return 0;

    return 1;
}
//------------------------------------------------------------------------------
void GetShortestDistance(struct QR_Vector3* pL1S, struct QR_Vector3* pL1E,
                         struct QR_Vector3* pL2S, struct QR_Vector3* pL2E, float* pEpsylon, float* pR)
{
    // calculate p2 - p1, p4 - p3, and p1 - p3 line segments
    struct QR_Vector3 delta21;
    struct QR_Vector3 delta43;
    struct QR_Vector3 delta13;
    Sub(pL1E, pL1S, &delta21);
    Sub(pL2E, pL2S, &delta43);
    Sub(pL1S, pL2S, &delta13);

    // calculate some needed values - a, c and D are always >= 0
    float a;
    float b;
    float c;
    float d;
    float e;
    Dot(&delta21, &delta21, &a);
    Dot(&delta21, &delta43, &b);
    Dot(&delta43, &delta43, &c);
    Dot(&delta21, &delta13, &d);
    Dot(&delta43, &delta13, &e);
    float D = ((a * c) - (b * b));

    // sc = sN / sD, default sD = D >= 0
    float sc;
    float sN;
    float sD = D;

    // tc = tN / tD, default tD = D >= 0
    float tc;
    float tN;
    float tD = D;

    // compute the line parameters of the two closest points
    if (D < *pEpsylon)
    {
        // the lines are almost parallel, force using point P0 on segment S1
        // to prevent possible division by 0 later
        sN = 0.0;
        sD = 1.0;
        tN = e;
        tD = c;
    }
    else
    {
        // get the closest points on the infinite lines
        sN = ((b * e) - (c * d));
        tN = ((a * e) - (b * d));

        // sc < 0 => the s=0 edge is visible
        if (sN < 0.0)
        {
            sN = 0.0;
            tN = e;
            tD = c;
        }
        else
        // sc > 1 => the s=1 edge is visible
        if (sN > sD)
        {
            sN = sD;
            tN = e + b;
            tD = c;
        }
    }

    // tc < 0 => the t=0 edge is visible
    if (tN < 0.0)
    {
        tN = 0.0;

        // recompute sc for this edge
        if (-d < 0.0)
            sN = 0.0;
        else
        if (-d > a)
            sN = sD;
        else
        {
            sN = -d;
            sD =  a;
        }
    }
    else
    // tc > 1 => the t=1 edge is visible
    if (tN > tD)
    {
        tN = tD;

        // recompute sc for this edge
        if ((-d + b) < 0.0)
            sN = 0;
        else
        if ((-d + b) > a)
            sN = sD;
        else
        {
            sN = (-d + b);
            sD = a;
        }
    }

    // finally do the division to get sc and tc
    if (fabs(sN) < *pEpsylon)
        sc = 0.0;
    else
        sc = sN / sD;

    if (fabs(tN) < *pEpsylon)
        tc = 0.0;
    else
        tc = tN / tD;

    struct QR_Vector3 dP;

    // get the difference of the two closest points
    dP.m_X = delta13.m_X + (sc * delta21.m_X) - (tc * delta43.m_X);
    dP.m_Y = delta13.m_Y + (sc * delta21.m_Y) - (tc * delta43.m_Y);
    dP.m_Z = delta13.m_Z + (sc * delta21.m_Z) - (tc * delta43.m_Z);

    // return the closest distance
    float dotdP;
    Dot(&dP, &dP, &dotdP);
    *pR = sqrt(dotdP);
}
//------------------------------------------------------------------------------
int Inside(struct QR_Vector3* pP, struct QR_Vector3* pV1, struct QR_Vector3* pV2, struct QR_Vector3* pV3)
{
    /*
    * here we check if the point P is inside the polygon
    *
    *              Collision                 No collision
    *                  V1                         V1
    *                  /\                         /\
    *                 /  \                       /  \
    *                / *P \                  *P /    \
    *               /      \                   /      \
    *            V2 -------- V3             V2 -------- V3
    *
    * we calculate the segments between the P point and each vertex of the
    * polygon and we normalize this segment. For that we uses the following
    * algorithms:
    * -> PToV1 = Polygon.Vertex1 - Point;
    * -> PToV2 = Polygon.Vertex2 - Point;
    * -> PToV3 = Polygon.Vertex3 - Point;
    */
    struct QR_Vector3 nPToV1;
    struct QR_Vector3 nPToV2;
    struct QR_Vector3 nPToV3;
    Sub(pV1, pP, &nPToV1);
    Sub(pV2, pP, &nPToV2);
    Sub(pV3, pP, &nPToV3);
    Normalize(&nPToV1, &nPToV1);
    Normalize(&nPToV2, &nPToV2);
    Normalize(&nPToV3, &nPToV3);

    // calculate the angles using the scalar product of each vectors. We use
    // the following algorithms:
    // A1 = NPToV1.x * NPToV2.x + NPToV1.y * NPToV2.y + NPToV1.z * NPToV2.z
    // A2 = NPToV2.x * NPToV3.x + NPToV2.y * NPToV3.y + NPToV2.z * NPToV3.z
    // A3 = NPToV3.x * NPToV1.x + NPToV3.y * NPToV1.y + NPToV3.z * NPToV1.z
    float a1;
    float a2;
    float a3;
    Dot(&nPToV1, &nPToV2, &a1);
    Dot(&nPToV2, &nPToV3, &a2);
    Dot(&nPToV3, &nPToV1, &a3);

    // calculate the sum of all angles
    float angleResult = acos(a1) + acos(a2) + acos(a3);

    // if sum is equal or higher to 6.28 radians then point P is inside polygon
    if (angleResult >= 6.28f)
        return 1;

    return 0;
}
//------------------------------------------------------------------------------
int Intersect(struct QR_Vector3* pP1V1, struct QR_Vector3* pP1V2, struct QR_Vector3* pP1V3,
              struct QR_Vector3* pP2V1, struct QR_Vector3* pP2V2, struct QR_Vector3* pP2V3)
{
    float epsylon = 1.0E-3;

    // get planes from polygons
    struct QR_Plane plane1;
    struct QR_Plane plane2;
    PlaneFromPoints(pP1V1, pP1V2, pP1V3, &plane1);
    PlaneFromPoints(pP2V1, pP2V2, pP2V3, &plane2);

    // are planes merged?
    if (((plane1.m_A ==  plane2.m_A)  &&
         (plane1.m_B ==  plane2.m_B)  &&
         (plane1.m_C ==  plane2.m_C)  &&
         (plane1.m_D ==  plane2.m_D)) ||
        ((plane1.m_A == -plane2.m_A)  &&
         (plane1.m_B == -plane2.m_B)  &&
         (plane1.m_C == -plane2.m_C)  &&
         (plane1.m_D == -plane2.m_D)))
    {
        // is any vertex inside other polygon?
        if (Inside(pP1V1, pP2V1, pP2V2, pP2V3) ||
            Inside(pP1V2, pP2V1, pP2V2, pP2V3) ||
            Inside(pP1V3, pP2V1, pP2V2, pP2V3) ||
            Inside(pP2V1, pP1V1, pP1V2, pP1V3) ||
            Inside(pP2V2, pP1V1, pP1V2, pP1V3) ||
            Inside(pP2V3, pP1V1, pP1V2, pP1V3))
            return 1;

        struct QR_Vector3 v1v2LineS;
        struct QR_Vector3 v1v2LineE;
        struct QR_Vector3 v2v3LineS;
        struct QR_Vector3 v2v3LineE;
        struct QR_Vector3 v3v1LineS;
        struct QR_Vector3 v3v1LineE;
        struct QR_Vector3 ov1ov2LineS;
        struct QR_Vector3 ov1ov2LineE;
        struct QR_Vector3 ov2ov3LineS;
        struct QR_Vector3 ov2ov3LineE;
        struct QR_Vector3 ov3ov1LineS;
        struct QR_Vector3 ov3ov1LineE;

        // create polygon lines
        Copy(pP1V1, &v1v2LineS);
        Copy(pP1V2, &v1v2LineE);
        Copy(pP1V2, &v2v3LineS);
        Copy(pP1V3, &v2v3LineE);
        Copy(pP1V3, &v3v1LineS);
        Copy(pP1V1, &v3v1LineE);
        Copy(pP2V1, &ov1ov2LineS);
        Copy(pP2V2, &ov1ov2LineE);
        Copy(pP2V2, &ov2ov3LineS);
        Copy(pP2V3, &ov2ov3LineE);
        Copy(pP2V3, &ov3ov1LineS);
        Copy(pP2V1, &ov3ov1LineE);

        float result;

        GetShortestDistance(&v1v2LineS,   &v1v2LineE,
                            &ov1ov2LineS, &ov1ov2LineE, &epsylon, &result);

        // is shortest distance between lines equal to 0?
        if (result < epsylon)
            return 1;

        GetShortestDistance(&v2v3LineS,   &v2v3LineE,
                            &ov1ov2LineS, &ov1ov2LineE, &epsylon, &result);

        // is shortest distance between lines equal to 0?
        if (result < epsylon)
            return 1;

        GetShortestDistance(&v3v1LineS,   &v3v1LineE,
                            &ov1ov2LineS, &ov1ov2LineE, &epsylon, &result);

        // is shortest distance between lines equal to 0?
        if (result < epsylon)
            return 1;

        GetShortestDistance(&v1v2LineS,   &v1v2LineE,
                            &ov2ov3LineS, &ov2ov3LineE, &epsylon, &result);

        // is shortest distance between lines equal to 0?
        if (result < epsylon)
            return 1;

        GetShortestDistance(&v2v3LineS,   &v2v3LineE,
                            &ov2ov3LineS, &ov2ov3LineE, &epsylon, &result);

        // is shortest distance between lines equal to 0?
        if (result < epsylon)
            return 1;

        GetShortestDistance(&v3v1LineS,   &v3v1LineE,
                            &ov2ov3LineS, &ov2ov3LineE, &epsylon, &result);

        // is shortest distance between lines equal to 0?
        if (result < epsylon)
            return 1;

        GetShortestDistance(&v1v2LineS,   &v1v2LineE,
                            &ov3ov1LineS, &ov3ov1LineE, &epsylon, &result);

        // is shortest distance between lines equal to 0?
        if (result < epsylon)
            return 1;

        GetShortestDistance(&v2v3LineS,   &v2v3LineE,
                            &ov3ov1LineS, &ov3ov1LineE, &epsylon, &result);

        // is shortest distance between lines equal to 0?
        if (result < epsylon)
            return 1;

        GetShortestDistance(&v3v1LineS,   &v3v1LineE,
                            &ov3ov1LineS, &ov3ov1LineE, &epsylon, &result);

        // is shortest distance between lines equal to 0?
        if (result < epsylon)
            return 1;

        return 0;
    }

    // get plane normals
    struct QR_Vector3 normal1;
    normal1.m_X = plane1.m_A;
    normal1.m_Y = plane1.m_B;
    normal1.m_Z = plane1.m_C;

    struct QR_Vector3 normal2;
    normal2.m_X = plane2.m_A;
    normal2.m_Y = plane2.m_B;
    normal2.m_Z = plane2.m_C;

    // calculate angle between planes
    float planesDot;
    Dot(&normal1, &normal2, &planesDot);

    // are plane parallels?
    if (planesDot == 1.0f || planesDot == -1.0f)
        // planes are parallels but not merged, no collision is possible
        return 0;

    // calculate distance from each first polygon vertex to second polygon plane
    float dist1v1;
    float dist1v2;
    float dist1v3;
    DistanceToPlane(pP1V1, &plane2, &dist1v1);
    DistanceToPlane(pP1V2, &plane2, &dist1v2);
    DistanceToPlane(pP1V3, &plane2, &dist1v3);

    // prepare list containing first polygon intersection points
    struct QR_Vector3 p1pt1;
    struct QR_Vector3 p1pt2;
    struct QR_Vector3 p1pt3;
    int        p1ptsCount = 0;

    // is first polygon V1 to V2 line segment intersects second polygon plane?
    if ((dist1v1 >= 0.0f && dist1v2 < 0.0f) || (dist1v1 < 0.0f && dist1v2 >= 0.0f))
    {
        struct QR_Vector3 p;

        // calculate intersection point and add to list on success
        if (PlaneIntersectLine(&plane2, pP1V1, pP1V2, &p))
        {
            Copy(&p, &p1pt1);
            ++p1ptsCount;
        }
    }

    // is first polygon V2 to V3 line segment intersects second polygon plane?
    if ((dist1v2 >= 0.0f && dist1v3 < 0.0f) || (dist1v2 < 0.0f && dist1v3 >= 0.0f))
    {
        struct QR_Vector3 p;

        // calculate intersection point and add to list on success
        if (PlaneIntersectLine(&plane2, pP1V2, pP1V3, &p))
        {
            Copy(&p, &p1pt2);
            ++p1ptsCount;
        }
    }

    // is first polygon V3 to V1 line segment intersects second polygon plane?
    if ((dist1v3 >= 0.0f && dist1v1 < 0.0f) || (dist1v3 < 0.0f && dist1v1 >= 0.0f))
    {
        struct QR_Vector3 p;

        // calculate intersection point and add to list on success
        if (PlaneIntersectLine(&plane2, pP1V3, pP1V1, &p))
        {
            Copy(&p, &p1pt3);
            ++p1ptsCount;
        }
    }

    // were the first polygon 2 intersection point found?
    if (p1ptsCount != 2)
        return 0;

    // calculate distance from each second polygon vertex to first polygon plane
    float dist2v1;
    float dist2v2;
    float dist2v3;
    DistanceToPlane(pP2V1, &plane1, &dist2v1);
    DistanceToPlane(pP2V2, &plane1, &dist2v2);
    DistanceToPlane(pP2V3, &plane1, &dist2v3);

    // prepare list containing second polygon intersection points
    struct QR_Vector3 p2pt1;
    struct QR_Vector3 p2pt2;
    struct QR_Vector3 p2pt3;
    int        p2ptsCount = 0;

    // is second polygon V1 to V2 line segment intersects first polygon plane?
    if ((dist2v1 >= 0.0f && dist2v2 < 0.0f) || (dist2v1 < 0.0f && dist2v2 >= 0.0f))
    {
        struct QR_Vector3 p;

        // calculate intersection point and add to list on success
        if (PlaneIntersectLine(&plane1, pP2V1, pP2V2, &p))
        {
            Copy(&p, &p2pt1);
            ++p2ptsCount;
        }
    }

    // is second polygon V2 to V3 line segment intersects first polygon plane?
    if ((dist2v2 >= 0.0f && dist2v3 < 0.0f) || (dist2v2 < 0.0f && dist2v3 >= 0.0f))
    {
        struct QR_Vector3 p;

        // calculate intersection point and add to list on success
        if (PlaneIntersectLine(&plane1, pP2V2, pP2V3, &p))
        {
            Copy(&p, &p2pt2);
            ++p2ptsCount;
        }
    }

    // is second polygon V3 to V1 line segment intersects first polygon plane?
    if ((dist2v3 >= 0.0f && dist2v1 < 0.0f) || (dist2v3 < 0.0f && dist2v1 >= 0.0f))
    {
        struct QR_Vector3 p;

        // calculate intersection point and add to list on success
        if (PlaneIntersectLine(&plane1, pP2V3, pP2V1, &p))
        {
            Copy(&p, &p2pt3);
            ++p2ptsCount;
        }
    }

    // were the second polygon 2 intersection point found?
    if (p2ptsCount != 2)
        return 0;

    // first and second polygon intersection points are on the same line, so
    // check if calculated first and second polygon segments intersect
    if (VectorIsBetween(&p1pt1, &p2pt1, &p2pt2, &epsylon))
        return 1;

    if (VectorIsBetween(&p1pt2, &p2pt1, &p2pt2, &epsylon))
        return 1;

    if (VectorIsBetween(&p2pt1, &p1pt1, &p1pt2, &epsylon))
        return 1;

    if (VectorIsBetween(&p2pt2, &p1pt1, &p1pt2, &epsylon))
        return 1;

    return 0;
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
void GetOrtho(float* pMaxX, float* pMaxY, struct QR_Matrix* pM)
{
    // calculate field of view values
    float a = 1.0f / *pMaxX;
    float b = 1.0f / *pMaxY;

    pM->m_Table[0][0] = a;    pM->m_Table[1][0] = 0.0f; pM->m_Table[2][0] =  0.0f; pM->m_Table[3][0] = 0.0f;
    pM->m_Table[0][1] = 0.0f; pM->m_Table[1][1] = b;    pM->m_Table[2][1] =  0.0f; pM->m_Table[3][1] = 0.0f;
    pM->m_Table[0][2] = 0.0f; pM->m_Table[1][2] = 0.0f; pM->m_Table[2][2] = -1.0f; pM->m_Table[3][2] = 0.0f;
    pM->m_Table[0][3] = 0.0f; pM->m_Table[1][3] = 0.0f; pM->m_Table[2][3] =  0.0f; pM->m_Table[3][3] = 1.0f;
}
//------------------------------------------------------------------------------
void ApplyMatrixToVector(struct QR_Matrix* pM, struct QR_Vector3* pV, struct QR_Vector3* pR)
{
    pR->m_X = (pV->m_X * pM->m_Table[0][0] +
               pV->m_Y * pM->m_Table[1][0] +
               pV->m_Z * pM->m_Table[2][0] +
                         pM->m_Table[3][0]);
    pR->m_Y = (pV->m_X * pM->m_Table[0][1] +
               pV->m_Y * pM->m_Table[1][1] +
               pV->m_Z * pM->m_Table[2][1] +
                         pM->m_Table[3][1]);
    pR->m_Z = (pV->m_X * pM->m_Table[0][2] +
               pV->m_Y * pM->m_Table[1][2] +
               pV->m_Z * pM->m_Table[2][2] +
                         pM->m_Table[3][2]);
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
    struct QR_Matrix ortho;
    GetOrtho(&maxX, &maxY, &ortho);

    // connect projection matrix to shader
    GLint projectionUniform = glGetUniformLocation(g_ShaderProgram, "qr_uProjection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &ortho.m_Table[0][0]);
}
//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    // delete shader program
    if (g_ShaderProgram)
        glDeleteProgram(g_ShaderProgram);

    g_ShaderProgram = 0;
}
//------------------------------------------------------------------------------
void on_GLES2_Size(int view_w, int view_h)
{
    glViewport(0, 0, view_w, view_h);
    ApplyOrtho(2.0f, 3.0f);
}
//------------------------------------------------------------------------------
void on_GLES2_Init(int view_w, int view_h)
{

    // compile, link and use shaders
    g_ShaderProgram = CompileShaders(pVertexShader, pFragmentShader);
    glUseProgram(g_ShaderProgram);
    
    // set viewport
    on_GLES2_Size(view_w, view_h);
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

    // get position and color slots
    GLuint positionSlot = glGetAttribLocation(g_ShaderProgram, "qr_aPosition");
    GLuint colorSlot    = glGetAttribLocation(g_ShaderProgram, "qr_aSourceColor");

    // enable position and color slots
    glEnableVertexAttribArray(positionSlot);
    glEnableVertexAttribArray(colorSlot);

    // set rotation axis
    struct QR_Vector3 r;
    r.m_X = 0.0f;
    r.m_Y = 0.0f;
    r.m_Z = 1.0f;

    struct QR_Matrix modelViewMatrixPoly1;
    struct QR_Matrix modelViewMatrixPoly2;
    GetIdentity(&modelViewMatrixPoly1);
    GetRotateMatrix(&g_Angle, &r, &modelViewMatrixPoly2);

    // get first polygon vertice 1
    struct QR_Vector3 p1t1;
    p1t1.m_X = firstPolygon_Red[0].m_Position[0];
    p1t1.m_Y = firstPolygon_Red[0].m_Position[1];
    p1t1.m_Z = firstPolygon_Red[0].m_Position[2];

    // get first polygon vertice 2
    struct QR_Vector3 p1t2;
    p1t2.m_X = firstPolygon_Red[1].m_Position[0];
    p1t2.m_Y = firstPolygon_Red[1].m_Position[1];
    p1t2.m_Z = firstPolygon_Red[1].m_Position[2];

    // get first polygon vertice 3
    struct QR_Vector3 p1t3;
    p1t3.m_X = firstPolygon_Red[2].m_Position[0];
    p1t3.m_Y = firstPolygon_Red[2].m_Position[1];
    p1t3.m_Z = firstPolygon_Red[2].m_Position[2];

    // get second polygon vertice 1
    struct QR_Vector3 p2t1;
    p2t1.m_X = secondPolygon_Red[0].m_Position[0];
    p2t1.m_Y = secondPolygon_Red[0].m_Position[1];
    p2t1.m_Z = secondPolygon_Red[0].m_Position[2];

    // get second polygon vertice 2
    struct QR_Vector3 p2t2;
    p2t2.m_X = secondPolygon_Red[1].m_Position[0];
    p2t2.m_Y = secondPolygon_Red[1].m_Position[1];
    p2t2.m_Z = secondPolygon_Red[1].m_Position[2];

    // get second polygon vertice 3
    struct QR_Vector3 p2t3;
    p2t3.m_X = secondPolygon_Red[2].m_Position[0];
    p2t3.m_Y = secondPolygon_Red[2].m_Position[1];
    p2t3.m_Z = secondPolygon_Red[2].m_Position[2];

    // rotate second polygon
    struct QR_Vector3 p2t1r;
    struct QR_Vector3 p2t2r;
    struct QR_Vector3 p2t3r;
    ApplyMatrixToVector(&modelViewMatrixPoly2, &p2t1, &p2t1r);
    ApplyMatrixToVector(&modelViewMatrixPoly2, &p2t2, &p2t2r);
    ApplyMatrixToVector(&modelViewMatrixPoly2, &p2t3, &p2t3r);

    // check if polygon 1 intersects polygon 2
    int collisionDetected = Intersect(&p1t1, &p1t2, &p1t3, &p2t1r, &p2t2r, &p2t3r);

    // connect model view matrix to shader
    GLint modelviewUniform = glGetUniformLocation(g_ShaderProgram, "qr_uModelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrixPoly1.m_Table[0][0]);

    // calculate vertex stride
    GLsizei stride = sizeof(struct QR_Vertex);

    GLvoid* pCoords;
    GLvoid* pColors;
    GLsizei vertexCount;

    if (collisionDetected)
    {
        pCoords     = &firstPolygon_Red[0].m_Position[0];
        pColors     = &firstPolygon_Red[0].m_Color[0];
        vertexCount = sizeof(firstPolygon_Red) / sizeof(struct QR_Vertex);
    }
    else
    {
        pCoords     = &firstPolygon_Blue[0].m_Position[0];
        pColors     = &firstPolygon_Blue[0].m_Color[0];
        vertexCount = sizeof(firstPolygon_Blue) / sizeof(struct QR_Vertex);
    }

    // connect first polygon to shader
    glVertexAttribPointer(positionSlot, 3, GL_FLOAT, GL_FALSE, stride, pCoords);
    glVertexAttribPointer(colorSlot,    4, GL_FLOAT, GL_FALSE, stride, pColors);

    // draw it
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    // connect model view matrix to shader
    glUniformMatrix4fv(modelviewUniform, 1, 0, &modelViewMatrixPoly2.m_Table[0][0]);

    if (collisionDetected)
    {
        pCoords     = &secondPolygon_Red[0].m_Position[0];
        pColors     = &secondPolygon_Red[0].m_Color[0];
        vertexCount = sizeof(secondPolygon_Red) / sizeof(struct QR_Vertex);
    }
    else
    {
        pCoords     = &secondPolygon_Blue[0].m_Position[0];
        pColors     = &secondPolygon_Blue[0].m_Color[0];
        vertexCount = sizeof(secondPolygon_Blue) / sizeof(struct QR_Vertex);
    }

    // connect first polygon to shader
    glVertexAttribPointer(positionSlot, 3, GL_FLOAT, GL_FALSE, stride, pCoords);
    glVertexAttribPointer(colorSlot,    4, GL_FLOAT, GL_FALSE, stride, pColors);

    // draw it
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    // disconnect slots from shader
    glDisableVertexAttribArray(positionSlot);
    glDisableVertexAttribArray(colorSlot);
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
