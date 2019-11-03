/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
precision mediump float;

uniform sampler2D u_texture;

varying vec2 vTexCoord;
varying vec4 fragColor;

void main(void)
{
    float dist = texture2D(u_texture, vTexCoord).a;
    float width = fwidth(dist);
    float alpha = smoothstep(0.5-width, 0.5+width, dist);
    gl_FragColor = vec4(fragColor.rgb, alpha*fragColor.a);
}


