// Fragment shader
#version 330

#define RAYCAST_MODE_MIP 0
#define RAYCAST_MODE_COMPOSITE 1
#define RAYCAST_MODE_ISOSURFACE 2

// Ouput data (G-buffer)
layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gColor;


uniform sampler3D u_volumeTexture;
uniform sampler2D u_backFaceTexture;
uniform sampler2D u_frontFaceTexture;
uniform bool u_useGammaCorrec;
uniform int u_maxSteps;
uniform float u_isoValue;
uniform mat4 u_matM;
uniform mat4 u_matV;
uniform mat4 u_matP;


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


// 6-voxel neigborhood Sobel filter
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
	// init B-buffer values to zero
	gPosition = vec4(0.0);
	gColor = vec4(0.0);
	gNormal = vec4(0.0);

    vec4 color = vec4(0.0);
	mat4 matMVP = u_matP * u_matV * u_matM;     // assemble model-viw-projection matrix
	float u_stepSize = 1.732/float(u_maxSteps); //1.732 = sqrt(3) = diag length

    vec4 frontFace = texture(u_frontFaceTexture, v_texcoord);
    vec4 backFace = texture(u_backFaceTexture, v_texcoord);

    if (frontFace.a == 0.0 || backFace.a == 0) { discard; }

    vec3 rayStart = frontFace.xyz;
    vec3 rayStop = backFace.xyz;
    vec3 rayDir = normalize(rayStop - rayStart);
	int numSteps = int(length(rayStart - rayStop) / u_stepSize );

	//vec4 Nreturn = vec4(0.0, 0.0, 0.0, 1.0); // normal to write in output texture
	//vec4 Preturn = vec4(0.0, 0.0, 0.0, 1.0); // position to write in output texture
	vec3 L = vec3(0.0, 0.0, 1.0);            // light vector


    vec3 pos = rayStart;
    float intensity = 0.0;

	for (int i = 0; i < numSteps; ++i) 
	{
		intensity = texture(u_volumeTexture, pos).r;

		if (intensity >= u_isoValue)
		{
			break;
		}

		pos += u_stepSize * rayDir;
	}

	// shading of surface voxel
	if (intensity >= u_isoValue)
	{
		//improve accuracy of iso surface position
		pos = interval_bisection(pos, rayDir, u_stepSize);

		// normal vec in 3D texture space
		vec3 normal = normalize(-imageGradient(u_volumeTexture, pos));
		// normal in img space used for lighting
		vec3 N = normalize(mat3(matMVP) * normal);
		// normal in view space to write in B-buffer
		vec4 Nreturn = normalize(mat4(u_matV * u_matM) * vec4(normal.xyz, 1.0));
		
		// Blinn-Phong illumination
		vec3 diffuseColor = vec3(0.9, 0.9, 0.9) * max(0.0, dot(N, L));
		vec3 ambientColor = vec3(0.1, 0.1, 0.1);
		color.rgb = diffuseColor + ambientColor;
		color.a = 1.0;

		// write model space position coords into G-buffer
		vec4 Preturn = u_matM * vec4(pos.rgb, 1.0);
		gPosition = vec4(Preturn.rgb, 1.0);

		// write normal into G-buffer
		gNormal = vec4(Nreturn.xyz, 1.0);

	}


	if(u_useGammaCorrec)
		color.rgb = linearToGamma(color.rgb);
	
	// write final color into G-buffer
	gColor = vec4(color.rgb, 1.0);

    frag_color = color;
}
