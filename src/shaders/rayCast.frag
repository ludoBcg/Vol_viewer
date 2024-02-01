// Fragment shader
#version 150


uniform sampler3D u_volumeTexture;
uniform sampler2D u_backFaceTexture;
uniform sampler2D u_frontFaceTexture;
uniform sampler1D u_lookupTexture;
uniform sampler2D u_perlinTex;
uniform bool u_useJitter;
uniform bool u_useGammaCorrec;
uniform int u_modeVR; // MIP = 1, alpha blending = 2
uniform int u_maxSteps;
uniform mat4 u_matMVP;
uniform vec2 u_screenDims;
uniform float u_opacity;



in vec2 v_texcoord;

out vec4 frag_color;


vec2 perlinNoiseScale = vec2(u_screenDims[0] * 0.01, u_screenDims[1] * 0.01);

vec4 TF(in float intensity)
{ 
    vec4 color;
    color.rgb = mix(vec3(1.0, 0.25, 0.0), vec3(1.0, 1.0, 1.0), intensity);
    color.a = clamp(1.0 * intensity, 0.0, 1.0);

    return color;
}

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
    vec4 color = vec4(0.0);
	
	float stepSize = 1.732/float(u_maxSteps); //1.732 = sqrt(3) = diag length

    vec4 frontFace = texture(u_frontFaceTexture, v_texcoord);
    vec4 backFace = texture(u_backFaceTexture, v_texcoord);

	// sample random vec
	float randomVal = texture(u_perlinTex, v_texcoord * perlinNoiseScale).r;

    if (frontFace.a == 0.0 || backFace.a == 0) { discard; }

    vec3 rayStart = frontFace.xyz;
    vec3 rayStop = backFace.xyz;
    vec3 rayDir = normalize(rayStop - rayStart);
	int numSteps = int(length(rayStart - rayStop) / stepSize );
    vec4 accumMIP = vec4(0.0);
	vec4 accumAB = vec4(0.0);

    vec3 pos = rayStart;

	if (u_useJitter)
	{
		// add random length in ray direction to start position
		pos += randomVal * stepSize * rayDir;
	}

    float intensity = 0.0;

	for (int i = 0; i < numSteps; ++i) 
	{
		intensity = texture(u_volumeTexture, pos).r;
		//intensity = textureLod(u_volumeTexture, pos, 5.0).r;

		accumMIP = max(accumMIP, vec4(intensity, intensity, intensity, 1.0));

		vec4 tfColor = vec4(intensity, intensity, intensity, intensity);
		tfColor.a = clamp(1.0 * intensity, 0.0, 1.0);
		tfColor.a *= stepSize / u_opacity; // reduce the alpha when you accumulate too many layers
		accumAB.rgb += (tfColor.rgb * tfColor.a) * (1.0 - accumAB.a); // accumulate color (ponderated by reduced alpha) with a decreasing weight
		accumAB.a += tfColor.a * (1.0 - accumAB.a); //accumulate alpha with a decreasing weight			

		pos += stepSize * rayDir;
	}

	if (u_modeVR == 1) //MIP
		color.rgba += accumMIP;
	else if (u_modeVR == 2) // Alpha blending
		color.rgba += accumAB;

	color.a = 1.0;

	if(u_useGammaCorrec)
		color.rgb = linearToGamma(color.rgb);
	
    frag_color = color;
}
