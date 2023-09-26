// Fragment shader
#version 330


// UNIFORMS
uniform sampler3D u_volumeTexture;
uniform mat4 u_matTex;
uniform float u_brightness;
uniform bool u_useGammaCorrec;


// INPUT	
in vec3 vert_uvw;

// OUTPUT
out vec4 frag_color;



vec3 gammaToLinear(in vec3 color)
{
    return pow(color, vec3(2.2));
}

vec3 linearToGamma(in vec3 color)
{
    return pow(color, vec3(1.0 / 2.2));
}

void main()
{
	vec4 texCoords = u_matTex * vec4(vert_uvw.xyz, 1.0);
	
	float intensity = texture(u_volumeTexture, vec3(texCoords) ).r;

	vec4 color = vec4(intensity, intensity, intensity, 1.0);
	
	if(u_useGammaCorrec)
		color.rgb = linearToGamma(color.rgb);
	
    frag_color = color;

}
