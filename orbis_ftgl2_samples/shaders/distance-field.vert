/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
precision mediump float; 

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec4 u_color;

attribute vec3 vertex;
attribute vec2 tex_coord;
attribute vec4 color;

varying vec2 vTexCoord;

//use your own output instead of gl_FragColor
varying vec4 fragColor;

void main(void)
{
    vTexCoord.xy = tex_coord.xy;
    fragColor    = color * u_color;
    gl_Position       = u_projection*(u_view*(u_model*vec4(vertex,1.0)));
}
