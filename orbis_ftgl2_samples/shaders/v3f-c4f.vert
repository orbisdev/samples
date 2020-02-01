/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
precision mediump float;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

attribute vec3 vertex;
attribute vec4 color;

//use your own output instead of gl_FrontColor
varying vec4 fragColor;

void main()
{
    //gl_FrontColor = color;
    fragColor    = color;
    gl_Position = projection*(view*(model*vec4(vertex,1.0)));
}
