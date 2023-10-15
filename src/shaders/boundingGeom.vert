// Vertex header
#version 330

// VERTEX ATTRIBUTES
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec3 a_color;
layout(location = 3) in vec2 a_uv;
layout(location = 4) in vec3 a_tex3D;

// UNIFORMS
uniform mat4 u_matM;
uniform mat4 u_matV;
uniform mat4 u_matP;


// OUTPUT
out vec3 vert_uv;


void main()
{
	// compute Model-View-Projection matrix
	mat4 matMVP = u_matP * u_matV * u_matM;
	
    vert_uv = vec3(1.0) - a_position.xyz;
    gl_Position = matMVP * a_position;
}
