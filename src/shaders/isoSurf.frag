// Fragment shader
#version 330

#define RAYCAST_MODE_MIP 0
#define RAYCAST_MODE_COMPOSITE 1
#define RAYCAST_MODE_ISOSURFACE 2

// Ouput data
layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gColor;


uniform sampler3D u_volumeTexture;
uniform sampler2D u_backFaceTexture;
uniform sampler2D u_frontFaceTexture;
uniform bool u_useGammaCorrec;
uniform int u_maxSteps;
uniform float u_isoValue;
uniform mat4 u_matMVP;
uniform int u_useAO;



in vec2 v_texcoord;

out vec4 frag_color;



float aoFactor(vec3 pos, vec3 normal) 
{
	// TODO
	return 1;
}


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
	int numSteps = int(length(rayStart - rayStop) / u_stepSize );

	vec3 N = vec3(0.0, 0.0, 0.0);
	vec3 L = vec3(0.0, 0.0, 1.0);

    vec3 pos = rayStart;
    float intensity = 0.0;
	gPosition = vec3(0.0);


	for (int i = 0; i < numSteps; ++i) 
	{
		intensity = texture(u_volumeTexture, pos).r;

		if (intensity >= u_isoValue)
		{
			break;
		}

		pos += u_stepSize * rayDir;
	}
	if (intensity >= u_isoValue)
	{
		//improve accuracy of iso surface position
		pos = interval_bisection(pos, rayDir, u_stepSize);

		vec3 normal = normalize(-imageGradient(u_volumeTexture, pos));
		N = normalize(mat3(u_matMVP) * normal);
		vec3 diffuseColor = vec3(0.9, 0.9, 0.9) * max(0.0, dot(N, L));
		vec3 ambientColor = vec3(0.1, 0.1, 0.1);
		color.rgb = diffuseColor + ambientColor;
		color.a = 1.0;

		gPosition = pos;
	}


	if(u_useGammaCorrec)
		color.rgb = linearToGamma(color.rgb);
	
    frag_color = color;


	gColor = color.rgb;
	gNormal = N;
}
