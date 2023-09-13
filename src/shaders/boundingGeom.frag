// Fragment header
#version 330

// INPUT
in vec3 vert_uv;

// OUTPUT
out vec4 frag_color;

void main()
{
    frag_color = vec4(vert_uv, 1.0);
}
