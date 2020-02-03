/*
    sprite_shaders.h

    shaders used in sprite.c

    - one shared vertex shader
      locations for added: u_offset, u_p1_pos, u_time (unused)
      u_offset to displace UVs to select one rectangle from a bigger one (spritesheet)
      u_p1_pos to displace the rendered sprite

    - one fragment shader
      1. use default texture color (no change)
*/


/// 1. Vertex shaders
static const char *vs =
/// 1. use u_offset to select one sprite from one texture
    "precision mediump float; \
     attribute vec4  a_Position; \
     attribute vec2  a_TextureCoordinates; \
     varying   vec2  v_TextureCoordinates; \
     uniform   vec2  u_offset; \
     uniform   vec2  u_p1_pos; \
     uniform   float u_time; \
     void main(void) \
     { \
       v_TextureCoordinates  = a_TextureCoordinates; \
       v_TextureCoordinates += u_offset; \
       gl_Position           = a_Position \
                             + vec4(u_p1_pos.x + 1., u_p1_pos.y - 0.5, 0., 0.); \
     }";


/// 2. Fragment shaders
static const char *fs =
/// 1. default, use texture color
    "precision mediump float; \
     uniform   sampler2D u_TextureUnit; \
     varying   vec2      v_TextureCoordinates; \
     void main(void) \
     { \
       gl_FragColor = texture2D(u_TextureUnit, v_TextureCoordinates); \
     }";

