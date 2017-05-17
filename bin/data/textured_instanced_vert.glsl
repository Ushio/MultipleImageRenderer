#version 120

uniform mat4 u_modelview;
uniform mat4 u_proj;

// e.g. 128 / 1024
uniform vec2 u_uvtipstep;
uniform float u_aspect;

// geometry
attribute vec3 a_position;
attribute vec2 a_texcoord;
attribute vec3 a_normal;

// instance
attribute vec3 a_location;
attribute float a_scale;
attribute vec2 a_uv;
attribute float a_layer;

// varying vec4 v_color;
varying vec2 v_texcoord;
varying float v_layer;

void main()
{
    // gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
    gl_Position = u_proj * u_modelview * vec4(a_scale * vec3(u_aspect, 1.0, 1.0) * a_position + a_location, 1.0);

    /*
    vec3 L = normalize(vec3(1.0, 1.0, 1.0));
    vec3 N = quat_mul(a_rotation, a_normal);
    float lambert = abs(dot(N, L));
    float half_lambert = mix(lambert, 1.0, 0.7);
    */

    // v_color = vec4(1.0, 1.0, 1.0, 1.0);

    // v_texcoord = a_texcoord;
    v_texcoord = a_uv + a_texcoord * u_uvtipstep;
    v_layer = a_layer;
}
