#version 100
/*
  text_ani.vert shader

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

uniform mat4 model;
uniform mat4 view; 
uniform mat4 projection; 
uniform vec4 meta;     // ( g_Time, ani_status, TOTAL_ANI_FRAMES, ftype_num )

attribute vec3 vertex; 
attribute vec2 tex_coord; 
attribute vec4 color; 

varying vec2  vTexCoord; 
varying vec4  fragColor;
varying float frame;
// info(frame, TOTAL_ANI_FRAMES, iTime, state)
// varying vec4 unused;

float t(float a)
{
    return mod(a, .01);
}

void main(void)
{
    vTexCoord.xy = tex_coord.xy; 
    fragColor    = color;

    //if( meta.w == .0 ) 
    vec4  offset = vec4(0.05, -0.02, 0., 0.15);

    if( meta.w == .1 ) offset = vec4(0.); // TYPE_1 is not displaced
    if( meta.w == .2 ) offset = vec4(0.0, 0.3, 0., 0.);
    if( meta.w == .3 ) offset = vec4(0.0, 0.3, 0., 0.);

    vec4  step   = offset / meta.z;
          frame  = mod(meta.x, meta.z);
    vec4  p1     = projection*(view*(model*vec4(vertex, 1.))),
          p0     = p1 - offset;
    // unused    = vec4(frame, meta.z, meta.x, meta.y);

//vec2 info = vec2( mod(meta.x, meta.z), mod(meta.y, .02) );
//float type_num = mod(meta.y, .01);

    // follow ani_status
    if(meta.y >= .3) // OUT
    { 
        if( meta.w  < .2 ) p1   -= step * frame; // move (L) from final position
        if( meta.w == .2 ) p1   += step * frame /2.; // move (U) from final position
      //if( meta.w == .1 ) p1.y -= step.y * frame;
        gl_Position = p1; return;
    }
    if(meta.y >= .2) // DEFAULT
    { 
        if( meta.w == .1 ) { p1   +=     cos(frame * .2) * .01;  }
        if( meta.w == .2 ) { p1.y += abs(sin(frame * .1) * .1 ); }
        if( meta.w == .3 ) { p1.w -=     cos(frame * .1) * .005; }

        gl_Position = p1; return; // reflect same ft-gl program default
    }
    if(meta.y >= .1) // IN
    { 
        //if( meta.w == .0 ) {
        p0   += step * frame; // move (R) to final position
        //if( meta.w == .1 ) {
        //      p0.w += (-.5 / meta.z * frame); }//vec4(0.);//(2. + frame); }
        gl_Position = p0; return;
    }
    if(meta.y >= .0) { return; } // CLOSED
}
