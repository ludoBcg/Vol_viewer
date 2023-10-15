// Fragment shader
#version 330


// UNIFORMS
uniform sampler2D u_screenTex;
	
// INPUT	
in vec3 vert_uv;

// OUTPUT
out vec4 frag_color;


// MAIN
void main()
{
	// final color
	vec4 color = vec4(1.0f);
	color.rgb = texture(u_screenTex, vert_uv.xy).rgb;
		
	frag_color = color;
}

