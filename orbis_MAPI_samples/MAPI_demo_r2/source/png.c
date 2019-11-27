/*
    png to texture wrappers
*/

#include <assert.h>
#include <orbis2d.h>  // reuse orbis2d png wrappers and Orbis2dTexture type
//#include <debugnet.h>

#include  <GLES2/gl2.h>
#include  <EGL/egl.h>


static GLuint load_texture(
    const GLsizei width, const GLsizei height,
    const GLenum  type,  const GLvoid *pixels)
{
    // create new OpenGL texture
    GLuint texture_object_id;
    glGenTextures(1, &texture_object_id);
    assert(texture_object_id != 0);
 
    glBindTexture(GL_TEXTURE_2D, texture_object_id);

#if defined _DEMO_
    // set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // generate texture from bitmap data
    glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, pixels);

    // when using VBOs, we use this:
    glGenerateMipmap(GL_TEXTURE_2D); 

#elif defined _MAPI_
    // set texture filtering
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // set texture wrapping mode
    /*glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);*/

    // generate texture from bitmap data
    glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, pixels);

#endif

    glBindTexture(GL_TEXTURE_2D, 0);

    return texture_object_id;
}

GLuint load_png_asset_into_texture(const char* relative_path)
{
    assert(relative_path != NULL);

    Orbis2dTexture* texture_raw = orbis2dLoadPngFromHost_v2(relative_path);

    GLenum format = GL_RGBA;
    if (texture_raw->depth == 8) format = GL_RGB;

    const GLuint texture_object_id = load_texture(
        texture_raw->width, texture_raw->height, format, orbis2dTextureGetDataPointer(texture_raw));

    // delete buffers
    if(texture_raw)
        orbis2dDestroyTexture(texture_raw);

    return texture_object_id;
}
