/*
 * load .png, register as textures and display on GLES2
 * implementation by @theorywrong
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kernel.h>
#include <systemservice.h>
#include <orbisPad.h>
#include <sys/fcntl.h>
#include <ps4link.h>
#include <orbisGl.h>
#include <debugnet.h>
#include <orbis2d.h>  // reuse orbis2d png wrappers and Orbis2dTexture type
/*typedef struct Orbis2dTexture
{
    uint32_t *datap;
    unsigned int width;
    unsigned int height;
    unsigned int depth;
}Orbis2dTexture;*/


//------------------------------------------------------------------------------
GLuint programID = 0;

//------------------------------------------------------------------------------
typedef struct {
    char *name;
    GLuint textureID;
    Orbis2dTexture* texture_raw;
} ResTexture;

int res_textures_cnt = 0;
ResTexture *res_textures;

GLuint s_xyz_loc;
GLuint s_uv_loc;
GLuint s_sampler_loc;

//------------------------------------------------------------------------------
ResTexture* get_texture(char* name) {
    if (strlen(name)>255)
        return NULL;

    for (int i = 0; i < res_textures_cnt; i++) {
        if (strcmp(name, res_textures[i].name) == 0)
            return &res_textures[i];
    }

    return NULL;
}
//------------------------------------------------------------------------------
int png_register_texture(char* name, char* path) {
    debugNetPrintf(DEBUG,"[PKGLoader] Registering texture: %s (path: %s)\n", name, path);

    if (get_texture(name)) {
        debugNetPrintf(DEBUG,"[PKGLoader][png_register_texture] Texture already exist, skip ...\n");
        return 0;
    }

    Orbis2dTexture* texture_raw = orbis2dLoadPngFromHost_v2(path);
    if (!texture_raw) {
        debugNetPrintf(DEBUG,"[PKGLoader][png_register_texture] Texture can't be loaded, skip ...\n");
        return 0;
    }

    res_textures = (ResTexture*)realloc(res_textures, (res_textures_cnt + 1) * sizeof(ResTexture));
    res_textures[res_textures_cnt].texture_raw = texture_raw;

    GLenum format = GL_RGBA;
    if (texture_raw->depth == 8) {
        format = GL_RGB;
    }

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    if(texture_id > 0)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, texture_raw->width, texture_raw->height, 0, format, GL_UNSIGNED_BYTE, orbis2dTextureGetDataPointer(texture_raw));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    } else {
        return 0;
    }

    res_textures[res_textures_cnt].textureID = texture_id;

    strcpy(res_textures[res_textures_cnt].name, name);
    res_textures_cnt++;

    debugNetPrintf(DEBUG,"[PKGLoader][png_register_texture] Texture registered !\n");

    return 1;
}

//------------------------------------------------------------------------------
void on_GLES2_Size(int view_w, int view_h)
{
    glViewport(0, 0, view_w, view_h);
}

//------------------------------------------------------------------------------
//OpenGLES2 handlers : init , final , update , render , touch-input
void on_GLES2_Init(int view_w, int view_h)
{
    int ret = png_register_texture("test", "host0:tentacle.png");

    const GLchar s_vertex_shader_code[] =
        "attribute vec4 a_xyz;\n"
        "attribute vec2 a_uv;\n"
        "varying vec2 v_uv;\n"
        "\n"
        "void main() {\n"
        "    gl_Position = a_xyz;\n"
        "    v_uv = a_uv;\n"
        "}\n";

    const GLchar s_fragment_shader_code[] =
        "precision mediump float;\n"
        "varying vec2 v_uv;\n"
        "uniform sampler2D s_texture;\n"
        "\n"
        "void main() {\n"
        "    gl_FragColor = texture2D(s_texture, v_uv);\n"
        "}\n";

    GLuint vertexShader;
    GLuint fragmentShader;

    vertexShader = orbisGlCompileShader(GL_VERTEX_SHADER, s_vertex_shader_code);
    if (!vertexShader) {
        debugNetPrintf(DEBUG, "Error during compiling vertex shader !\n");
    }

    fragmentShader = orbisGlCompileShader(GL_FRAGMENT_SHADER, s_fragment_shader_code);
    if (!fragmentShader) {
        debugNetPrintf(DEBUG, "Error during compiling fragment shader !\n");
    }

    programID = orbisGlLinkProgram(vertexShader, fragmentShader);
    if (!programID) {
        debugNetPrintf(DEBUG, "Error during linking shader ! program_id=%d (0x%08x)\n", programID, programID);
    }

    s_xyz_loc     = glGetAttribLocation(programID,  "a_xyz");
    s_uv_loc      = glGetAttribLocation(programID,  "a_uv");
    s_sampler_loc = glGetUniformLocation(programID, "s_texture");

    glUseProgram(programID);
    ret = glGetError();
    if (ret) {
        debugNetPrintf(ERROR,"[ORBIS_GL] glUseProgram failed: 0x%08X\n", ret);
    }

    // set viewport
    on_GLES2_Size(view_w, view_h);
}

//------------------------------------------------------------------------------
int png_unregister_texture(char* name) {
    ResTexture* texture = get_texture(name);
    if (!texture)
        return 0;

    if (texture->texture_raw != NULL)
        orbis2dDestroyTexture(texture->texture_raw);

    debugNetPrintf(DEBUG,"[PKGLoader][png_unregister_texture] Texture unregistered !\n");

    memset(texture, 0, sizeof(ResTexture));

    return 1;
}

//------------------------------------------------------------------------------
void on_GLES2_Final()
{
    if (programID)
        glDeleteProgram(programID);
    programID = 0;

    png_unregister_texture("test");
}

//------------------------------------------------------------------------------
void on_GLES2_Update(float timeStep_sec)
{
}

//------------------------------------------------------------------------------
void draw_texture(char* name, int x, int y) {
    float fx = x;
    float fy = y;

    ResTexture* texture = get_texture(name);
    if (!texture)
        return;

    const GLfloat vertexArray[] = 
    {
        -0.5f,  0.5f, 0.0f, // Position 0
        -0.5f, -0.5f, 0.0f, // Position 1
         0.5f, -0.5f, 0.0f, // Position 2
         0.5f,  0.5f, 0.0f, // Position 3
    };

    const GLfloat textureArray[] = 
    {
        0.0f,  0.0f, // Position 0
        0.0f,  1.0f, // Position 1
        1.0f,  1.0f, // Position 2
        1.0f,  0.0f, // Position 3
    };

    const uint16_t drawList[] = { 0, 1, 2, 0, 2, 3 };

    int ret = 0;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glVertexAttribPointer(s_xyz_loc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)vertexArray);
    ret = glGetError();
    if (ret) {
        debugNetPrintf(ERROR,"[ORBIS_GL] glVertexAttribPointer failed: 0x%08X\n", ret);
    }

    glVertexAttribPointer(s_uv_loc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)textureArray);
    ret = glGetError();
    if (ret) {
        debugNetPrintf(ERROR,"[ORBIS_GL] glVertexAttribPointer failed: 0x%08X\n", ret);
    }

    glEnableVertexAttribArray(s_xyz_loc);
    ret = glGetError();
    if (ret) {
        debugNetPrintf(ERROR,"[ORBIS_GL] glEnableVertexAttribArray (1) failed: 0x%08X\n", ret);
    }

    glEnableVertexAttribArray(s_uv_loc);
    ret = glGetError();
    if (ret) {
        debugNetPrintf(ERROR,"[ORBIS_GL] glEnableVertexAttribArray (2) failed: 0x%08X\n", ret);
    }

    glActiveTexture(GL_TEXTURE0);
    ret = glGetError();
    if (ret) {
        debugNetPrintf(ERROR,"[PKGLoader] glActiveTexture failed: 0x%08X\n", ret);
    }

    glBindTexture(GL_TEXTURE_2D, texture->textureID);
    ret = glGetError();
    if (ret) {
        debugNetPrintf(ERROR,"[PKGLoader] glBindTexture failed: 0x%08X\n", ret);
    }

    glUniform1i(s_sampler_loc, 0);
    ret = glGetError();
    if (ret) {
        debugNetPrintf(ERROR,"[PKGLoader] glUniform1i failed: 0x%08X\n", ret);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, drawList);
    ret = glGetError();
    if (ret) {
        debugNetPrintf(ERROR,"[PKGLoader] glDrawElements failed: 0x%08X\n", ret);
    }
    // disconnect slots from shader
    glDisableVertexAttribArray(s_xyz_loc);
    glDisableVertexAttribArray(s_uv_loc);
}

//------------------------------------------------------------------------------
void on_GLES2_Render()
{
    // clear scene background and depth buffer
    glClearColor(0.7f, 0.7f, 0.7f, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    draw_texture("test", 10, 100);
}
