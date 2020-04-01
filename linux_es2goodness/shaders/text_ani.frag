#version 100
/*
  text_ani.frag shader

  2020, masterzorag

  enum text_ani_state, as ani_status stage:
  {
    CLOSED,
    IN,
    DEFAULT,
    OUT
  };
*/

precision mediump float;

uniform sampler2D texture;
uniform vec4      meta;  // g_Time, ani_status, TOTAL_ANI_FRAMES, type_num

varying vec2  vTexCoord;
varying vec4  fragColor;
varying float frame;

// info(frame, TOTAL_ANI_FRAMES, iTime, state)
// varying vec4 unused;

void main(void)
{
    float a      = texture2D(texture, vTexCoord).a;
    float step   = frame / meta.z;
    vec4  c1     = vec4( fragColor.rgb, fragColor.a*a );
        //c0     = vec4( c1.rgb, clamp(step, 0., c1.a ));
    gl_FragColor = c1;

    // follow ani_status
    if(meta.y >= .3) // OUT
    {
        step           = 1. - step; // reverse value
        gl_FragColor.a = clamp(step, 0., c1.a );

        if(meta.w == .1) gl_FragColor.rgb = vec3(1., .2, .2); // debug.r
        return;
    }
    if(meta.y >= .2) // DEFAULT
    {
        if(meta.w == .2 ) { gl_FragColor.rgb = vec3(.0, .2, 1.); }
        if(meta.w == .3 ) { gl_FragColor.a   = clamp(abs(sin(meta.x * .1)),
                                                     0., c1.a); }
        return; }

    if(meta.y >= .1) // IN
    { 
        if(meta.w == .1) gl_FragColor.rgb = vec3(.2, 1., .2); // debug.g

        gl_FragColor.a = clamp(step, 0., c1.a ); return;
    }
    if(meta.y >= .0) // CLOSED
    {
        gl_FragColor.a = 0.; return;
    }
}
