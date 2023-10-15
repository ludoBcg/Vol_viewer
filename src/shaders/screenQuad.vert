// Vertex shader
#version 330


// VERTEX ATTRIBUTES
layout(location = 0) in vec4 a_position;
layout(location = 3) in vec2 a_uv;


// OUTPUT
out vec3 vert_uv;


void main()
{
	gl_Position = a_position;
	
	vert_uv = vec3(a_uv.x, a_uv.y, 0.0);
}
