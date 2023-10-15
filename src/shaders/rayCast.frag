// Fragment shader
#version 150

#define RAYCAST_MODE_MIP 0
#define RAYCAST_MODE_COMPOSITE 1
#define RAYCAST_MODE_ISOSURFACE 2


uniform sampler3D u_volumeTexture;
uniform sampler2D u_backFaceTexture;
uniform sampler2D u_frontFaceTexture;
uniform sampler1D u_lookupTexture;
uniform float u_log;
uniform float u_brightness;
uniform bool u_useGammaCorrec;
uniform int u_modeVR; // MIP = 1, alpha blending = 2, isosurface = 3
uniform int u_maxSteps;
uniform float u_isoValue;


in vec2 v_texcoord;

out vec4 frag_color;

// Performs interval bisection that can be used to improve the
// accuracy of iso-surface detection. Based on a CG example in the
// SIGGRAPH2009 course notes on Advanced Illumination Techniques for
// GPU-Based Volume Raycasting.
vec3 interval_bisection(vec3 ray_position, vec3 ray_direction, float u_stepSize)
{
	// start = voxel pos - stepsize*raydir
	vec3 start = ray_position - u_stepSize * ray_direction;
	// end = voxel pos
	vec3 end = ray_position;
	int num_bisections = 4;
	for (int b = 0; b < num_bisections; ++b)
	{
		// test pos = middle between start and end
		vec3 test_position = (start + end) / 2.0;

		if (texture(u_volumeTexture, test_position).r < u_isoValue)
		{
			// if test pos is outside isosurface, use it as new start position
			start = test_position;
		}
		else
		{
			// if test pos is inside isosurface, use it as new start position
			end = test_position;
		}
	}

	return (start + end) / 2.0;
}


vec3 imageGradient(in sampler3D image, in vec3 pos)
{
    vec3 grad = vec3(0.0);
    grad.x += textureOffset(image, pos,  ivec3(1, 0, 0)).r;
    grad.x -= textureOffset(image, pos, -ivec3(1, 0, 0)).r;
    grad.y += textureOffset(image, pos,  ivec3(0, 1, 0)).r;
    grad.y -= textureOffset(image, pos, -ivec3(0, 1, 0)).r;
    grad.z += textureOffset(image, pos,  ivec3(0, 0, 1)).r;
    grad.z -= textureOffset(image, pos, -ivec3(0, 0, 1)).r;

    return grad;
}


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
	
	float u_stepSize = 1.732/float(u_maxSteps); //1.732 = sqrt(3) = diag length

    vec4 frontFace = texture(u_frontFaceTexture, v_texcoord);
    vec4 backFace = texture(u_backFaceTexture, v_texcoord);

    if (frontFace.a == 0.0 || backFace.a == 0) { discard; }

    vec3 rayStart = frontFace.xyz;
    vec3 rayStop = backFace.xyz;
    vec3 rayDir = normalize(rayStop - rayStart);
    int numSteps = int(min(u_maxSteps, length(rayStart - rayStop) / u_stepSize));
	

    vec4 accum = vec4(0.0);
    vec3 pos = rayStart;
    float intensity = 0.0;

	if(u_modeVR == 3) // isosurface
	{
		for (int i = 0; i < numSteps; ++i) 
		{
			intensity = texture(u_volumeTexture, pos).r;

			if (intensity >= u_isoValue)
			{
				//improve accuracy of iso surface position
				pos = interval_bisection(pos, rayDir, u_stepSize);

				vec3 normal = normalize(-imageGradient(u_volumeTexture, pos));
				//vec3 N = normalize(mat3(u_mv) * normal);
				vec3 N = normalize(normal);
				vec3 L = vec3(0.0, 0.0, 1.0);
				vec3 diffuseColor = N * 0.5 + 0.5;
				//accum.rgb = vec3(0.95, 0.95, 0.95) * max(0.0, dot(N, L));
				//accum.rgb = vec3(intensity, intensity, intensity);
				accum.rgb = diffuseColor;
				accum.a = 1.0;
				break;
			}

			pos += u_stepSize * rayDir;
		}
	}
	else // MIP or alpha blending
	{
		for (int i = 0; i < numSteps; ++i) 
		{
			intensity = texture(u_volumeTexture, pos).r;
			//intensity = textureLod(u_volumeTexture, pos, 5.0).r;

			if(u_modeVR == 1) //MIP
			{
				accum = max(accum, vec4(intensity, intensity, intensity, 1.0));
			}
			else if(u_modeVR == 2) // alpha blending
			{
				vec4 tfColor = vec4(intensity, intensity, intensity, intensity);
				tfColor.a = clamp(1.0 * intensity, 0.0, 1.0);
				tfColor.a *= u_stepSize / 0.02; // juste to reduce the alpha when you accumulate too many layers
				accum.rgb += (tfColor.rgb * tfColor.a) * (1.0 - accum.a); // accumulate color (ponderated by reduced alpha) with a decreasing weight
				accum.a += tfColor.a * (1.0 - accum.a); //accumulate alpha with a decreasing weight			
			}

			pos += u_stepSize * rayDir;
		}
	}
	

	color.rgba += accum;
	color.a = 1.0;

	if(u_useGammaCorrec)
		color.rgb = linearToGamma(color.rgb);
	
    frag_color = color;
}
