#version 100
/*
  text_ani.frag shader

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

uniform sampler2D texture;
uniform vec3      u_time;  // g_Time, ani_status, TOTAL_ANI_FRAMES

varying vec2  vTexCoord;
varying vec4  fragColor;
varying float frame;

// info(frame, TOTAL_ANI_FRAMES, iTime, state)
// varying vec4 unused;

void main(void)
{
    if(u_time.y >= .3) // CLOSED
    { gl_FragColor.a = 0.; return; }

    float a      = texture2D(texture, vTexCoord).a;
    float step   = frame / u_time.z;
    vec4  c1     = vec4( fragColor.rgb, fragColor.a*a );
        //c0     = vec4( c1.rgb, clamp(step, 0., c1.a ));
    gl_FragColor = c1;

    // follow ani_status
    if(u_time.y == .2) // DEFAULT
    { return; }

    if(u_time.y == .0) // IN
    { gl_FragColor.a = clamp(step, 0., c1.a ); return; }

    if(u_time.y == .1) // OUT
    { step           = 1. - step; // reverse value
      gl_FragColor.a = clamp(step, 0., c1.a ); return; }
}
