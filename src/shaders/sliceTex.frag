// Fragment shader
#version 330


// UNIFORMS
uniform sampler3D u_volumeTexture;
uniform sampler1D u_lookupTexture;
uniform mat4 u_matTex;
uniform float u_log;
uniform float u_brightness;
uniform bool u_useGammaCorrec;
uniform float u_threshMin;
uniform float u_threshMax;


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

float log10(in float x)
{
	float fact = 0.434294; // constant factor = (1.0 / log(10.0))
	return fact * log(x);
}

void main()
{
	vec4 texCoords = u_matTex * vec4(vert_uvw.xyz, 1.0);
	
	float intensity = texture(u_volumeTexture, vec3(texCoords) ).r;
	
	intensity = min(u_threshMax, intensity);
	if(intensity < u_threshMin) { intensity = 0.0; };

	vec4 color = vec4(intensity, intensity, intensity, 1.0);
	
	if(u_useGammaCorrec)
		color.rgb = linearToGamma(color.rgb);
	
    frag_color = color;

}
