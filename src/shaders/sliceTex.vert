// Vertex shader
#version 330
#extension GL_ARB_explicit_attrib_location : require

// VERTEX ATTRIBUTES
layout(location = 0) in vec4 a_position;
layout(location = 4) in vec3 a_tex3D;

// UNIFORMS
uniform mat4 u_matM;
uniform mat4 u_matV;
uniform mat4 u_matP;

// OUTPUT
out vec3 vert_uvw;

void main()
{
	// compute Model-View and Model-View-Projection matrices
	mat4 matMV = u_matV * u_matM;
	mat4 matMVP = u_matP * matMV;

	gl_Position = matMVP * a_position;
	
	vert_uvw = a_tex3D;
	
}
