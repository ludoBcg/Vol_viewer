// Fragment shader
#version 150


uniform sampler3D u_volumeTexture;
uniform sampler2D u_backFaceTexture;
uniform sampler2D u_frontFaceTexture;
uniform sampler1D u_lookupTexture;
uniform bool u_useGammaCorrec;
uniform int u_useTF;
uniform int u_modeVR; // MIP = 1, alpha blending = 2
uniform int u_maxSteps;
uniform mat4 u_matMVP;
uniform float u_transparency;



in vec2 v_texcoord;

out vec4 frag_color;


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

    if (frontFace.a == 0.0 || backFace.a == 0) { discard; }

    vec3 rayStart = frontFace.xyz;
    vec3 rayStop = backFace.xyz;
    vec3 rayDir = normalize(rayStop - rayStart);
	int numSteps = int(length(rayStart - rayStop) / stepSize );
    vec4 accumMIP = vec4(0.0);
	vec4 accumAB = vec4(0.0);

    vec3 pos = rayStart;

    float intensity = 0.0;

	for (int i = 0; i < numSteps && accumAB.a < 1.0; ++i)
	{
		intensity = texture(u_volumeTexture, pos).r;
		//intensity = textureLod(u_volumeTexture, pos, 5.0).r;

		accumMIP = max(accumMIP, vec4(intensity, intensity, intensity, 1.0));

		// read color from TF
		vec3 material = texture(u_lookupTexture, intensity).rgb * u_useTF
								+ vec3(intensity, intensity, intensity) * (1 - u_useTF);

		vec4 tfColor = vec4(material.rgb, intensity);
		tfColor.a = clamp(1.0 * intensity, 0.0, 1.0);
		tfColor.a *= stepSize / u_transparency; // reduce the alpha when you accumulate too many layers
		accumAB.rgb += (tfColor.rgb * tfColor.a) * (1.0 - accumAB.a); // accumulate color (ponderated by reduced alpha) with a decreasing weight
		accumAB.a += tfColor.a * (1.0 - accumAB.a); //accumulate alpha with a decreasing weight

		pos += stepSize * rayDir;
	}

	if (u_modeVR == 1) //MIP
		color.rgba += accumMIP;
	else if (u_modeVR == 2) // Alpha blending
		color.rgba += accumAB;

	color.a = 1.0;

	//color.rgb = texture(u_lookupTexture, color.r).rgb;

	if(u_useGammaCorrec)
		color.rgb = linearToGamma(color.rgb);
	
    frag_color = color;
}
