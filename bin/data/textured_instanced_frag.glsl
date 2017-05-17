#version 120
#extension GL_EXT_texture_array : enable

uniform sampler2DArray u_texture;

// varying vec4 v_color;
varying vec2 v_texcoord;
varying float v_layer;


void main()
{
    vec4 color = texture2DArray(u_texture, vec3(v_texcoord, v_layer));
    if(color.a < 0.01) {
        discard;
    }
    gl_FragColor = vec4(color.rgb, color.a);
}
