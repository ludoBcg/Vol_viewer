// Vertex shader
#version 330
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec4 a_position;

out vec2 v_texcoord;


void main()
{
    v_texcoord = 0.5 * a_position.xy + 0.5;
    gl_Position = a_position;
}
