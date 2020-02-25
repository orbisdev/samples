#version 100
/*
  text_ani.vert shader

  2020, masterzorag

  enum text_ani_state, as ani_status stage:
  {
    IN,
    OUT,
    DEFAULT,
    CLOSED
  };
*/

precision mediump float;

uniform mat4 model;
uniform mat4 view; 
uniform mat4 projection; 
uniform vec3 u_time;     // ( g_Time, ani_status, TOTAL_ANI_FRAMES )

attribute vec3 vertex; 
attribute vec2 tex_coord; 
attribute vec4 color; 

varying vec2  vTexCoord; 
varying vec4  fragColor;
varying float frame;
// info(frame, TOTAL_ANI_FRAMES, iTime, state)
// varying vec4 unused;

void main(void)
{
    if(u_time.y >= .3) { return; } // CLOSED

    vTexCoord.xy = tex_coord.xy; 
    fragColor    = color;

    vec4  offset = vec4(0.05, -0.02, 0., 0.15);
    vec4 step    = offset / u_time.z;
          frame  = mod(u_time.x, u_time.z);
    vec4  p1     = projection*(view*(model*vec4(vertex, 1.))),
          p0     = p1 - offset;
    // unused    = vec4(frame, u_time.z, u_time.x, u_time.y);

    // follow ani_status
    if(u_time.y == .2) // DEFAULT
    { gl_Position = p1; return; } // reflect same ft-gl program default

    if(u_time.y == .0) // IN
    { p0         += step * frame; // move (R) to final position
      gl_Position = p0; return; }

    if(u_time.y == .1) // OUT
    { p1         -= step * frame; // move (L) from final position
      gl_Position = p1; return; }
}